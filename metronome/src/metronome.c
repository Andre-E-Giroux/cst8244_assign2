#include <stdio.h>
#include <stdlib.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL
typedef union {
	struct _pulse pulse;
	char msg[255];
} my_message_t;

const int timeTable[8][4] = { { 2, 4, 4 }, { 3, 4, 6 }, { 4, 4, 8 },
		{ 5, 4, 10 }, { 3, 8, 6 }, { 6, 8, 6 }, { 9, 8, 9 }, { 12, 8, 12 } };

const char *outputTable[] = { "|1&2&", "|1&2&3&", "|1&2&3&4&", "|1&2&3&4-5-",
		"|1-2-3-", "|1&a2&a", "|1&a2&a3&a", "|1&a2&a3&a4&a" };
const int stopCode = 29;
const int startCode = 30;
const int quitCode = -1;

int bpm;
volatile int quit = 0;
int tst;
int tsb;
int outputPosition;
int server_coid;
char data[255];
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {

	int nb;
	if (data == NULL)
		return 0;
	nb = strlen(data);

	//test to see if we have already sent the whole message.
	if (ocb->offset == nb)
		return 0;

	//We will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, data, nb);

	//update offset into our data used to determine start position for next read.
	ocb->offset += nb;

	//If we are going to send any bytes update the access time for this resource.
	if (nb > 0)
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
	int nb = 0;

	if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {
		/* have all the data */
		char *buf;
		char *alert_msg;
		int i, small_integer;
		buf = (char *) (msg + 1);

		if (strstr(buf, "pause") != NULL) {
			for (i = 0; i < 2; i++) {
				alert_msg = strsep(&buf, " ");
			}
			small_integer = atoi(alert_msg);
			if (small_integer >= 1 && small_integer <= 9) {
				//FIXME :: replace getprio() with SchedGet()
				MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
				_PULSE_CODE_MINAVAIL, small_integer);
			} else {
				printf("Integer is not between 1 and 9.\n");
			}
		} else if (strstr(buf, "stop") != NULL) {

			MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
			_PULSE_CODE_MINAVAIL, stopCode);
		} else if (strstr(buf, "start") != NULL) {

			MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
			_PULSE_CODE_MINAVAIL, startCode);
		} else if (strstr(buf, "quit") != NULL) {

			MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
			_PULSE_CODE_MINAVAIL, quitCode);
		} else {
			strcpy(data, buf);
		}

		nb = msg->i.nbytes;
	}
	_IO_SET_WRITE_NBYTES(ctp, nb);

	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS(0));
}

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra) {
	if ((server_coid = name_open("metronome", 0)) == -1) {
		perror("name_open failed.");
		return EXIT_FAILURE;
	}
	return (iofunc_open_default(ctp, msg, handle, extra));
}
void set(char *argv[]) {
	int tempBpm = atoi(argv[1]);
	int tempTst = atoi(argv[2]);
	int tempTsb = atoi(argv[3]);
	for (int i = 0; i < 8; i++) {

		if (timeTable[i][0] == tempTst && timeTable[i][1] == tempTsb) {
			bpm = tempBpm;
			tst = tempTst;
			tsb = tempTsb;
			outputPosition = i;
			return;
		}
	}
	printf("Invalid timings.\n");
	return;
}

void *childThread() {
	name_attach_t *attach;
	my_message_t msg;
	int rcvid;
	int stopped = 0;
	char command[255];
	char inBpm[10];
	char inTst[3];
	char inTsb[3];
	FILE * fp;
	attach = name_attach(NULL, "metronome", 0);
	if (attach == NULL) {
		perror("name_attach failed");
		pthread_exit(NULL);
	}
	fp = fopen("dev/local/metronome", "r");
	if (fp == NULL) {
		perror("fopen failed");
		pthread_exit(NULL);
	}

	fscanf(fp, "%s", command);

	if (strcmp(command, "set") == 0) {
		fscanf(fp, "%s", inBpm);
		fscanf(fp, "%s", inTst);
		fscanf(fp, "%s", inTsb);
		char *input[] = { inBpm, inTst, inTsb };
		set(input);
	}

	fclose(fp);

	while (1) {
		rcvid = MsgReceivePulse(attach->chid, &msg, sizeof(msg), NULL);
		if (rcvid == 0) {

			if (msg.pulse.code == MY_PULSE_CODE) { //ASK PROF ABOUT THIS
				fp = fopen("dev/local/metronome", "r");
				if (fp == NULL) {
					perror("fopen failed");
					pthread_exit(NULL);
				}

				fscanf(fp, "%s", command);
				if (stopped == 0) {
					if (strcmp(command, "set") == 0) {
						fscanf(fp, "%s", inBpm);
						fscanf(fp, "%s", inTst);
						fscanf(fp, "%s", inTsb);
						char *input[] = { inBpm, inTst, inTsb };
						set(input);
					}
					if (msg.pulse.value.sival_int<10) {

						int sleepTime = msg.pulse.value.sival_int;
						sleep(sleepTime);
					}
					if (msg.pulse.value.sival_int==stopCode) {
						stopped = 1;
					}

				}
				if (msg.pulse.value.sival_int==startCode) {
					stopped = 0;
				}
				if (msg.pulse.value.sival_int==quitCode) {
					name_detach(attach, 0);
					quit = 1;
					pthread_exit(NULL);
				}
				fclose(fp);
			}
		} else {
			perror("Error recieving pulse message");
			name_detach(attach, 0);
			pthread_exit(NULL);
		}
	}
	name_detach(attach, 0);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	if (argc != 1 && argc != 4) {
		printf("Invalid Number Of Arguments\n");
		return EXIT_FAILURE;
	}

	if (argc == 4) {
		set(argv);
		return EXIT_SUCCESS;
	}
	dispatch_t* dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	iofunc_attr_t ioattr;
	dispatch_context_t *ctp;
	int id;

	if ((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s:  Unable to allocate dispatch context.\n", argv[0]);
		return (EXIT_FAILURE);
	}
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS,
			&io_funcs);
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);

	if ((id = resmgr_attach(dpp, NULL, "/dev/local/metronome", _FTYPE_ANY, 0,
			&connect_funcs, &io_funcs, &ioattr)) == -1) {
		fprintf(stderr, "%s:  Unable to attach name.\n", argv[0]);
		return (EXIT_FAILURE);
	}

	ctp = dispatch_context_alloc(dpp);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(NULL, &attr, *childThread, NULL);
	pthread_attr_destroy(&attr);
	while (quit == 0) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}
	return EXIT_SUCCESS;
}
