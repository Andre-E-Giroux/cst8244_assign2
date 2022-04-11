#!/bin/sh
#######################################################################
## - acceptance-test-ass2.sh
## - Assignment 2 - Metronome
## - @reference - item b) on page 7 of assignment specification
## - @author giro0172@algonquinlive.com
#######################################################################

echo "A\n********************************************"
echo "/metronome "
echo "Expected: usage message"
./metronome


echo "B\n********************************************"
echo "./metronome 120 2 4 &"
echo "Expected: 1 measure per second."
./metronome 120 2 4 &

echo "C\n********************************************"
echo "echo /dev/local/metronome"
echo "Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
cat /dev/local/metronome

echo "D\n********************************************"
echo "echo /dev/local/metronome-help"
echo "Expected:information regarding the metronome resmgr’s API, as seen above."
cat /dev/local/metronome-help

echo "E\n********************************************"
echo "echo set 100 2 4 > /dev/local/metronome"
echo "Expected: metronome regmgr changes settings to: 200 bpm in 5/4 time; run-time behaviour of metronome changes to 200 bpm in 5/4 time"
echo set 100 2 4 > /dev/local/metronome

echo "F\n********************************************"
echo "echo /dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per- interval: 0.15, nanoSecs: 150000000]"
cat /dev/local/metronome

echo "G\n********************************************"
echo "echo set 200 5 4 > /dev/local/metronome"
echo "Expected: metronome regmgr changes settings to: 200 bpm in 5/4 time; run-time behaviour of metronome changes to 200 bpm in 5/4 time"
echo set 200 5 4 > /dev/local/metronome

echo "H\n********************************************"
echo "cat /dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per- interval: 0.15, nanoSecs: 150000000]"
cat /dev/local/metronome


echo "I\n********************************************"
echo "echo stop > /dev/local/metronome"
echo "Expected: metronome stops running; metronome resmgr is still running as a process: pidin | grep metronome"
echo stop > /dev/local/metronome

echo "J\n********************************************"
echo "echo start > /dev/local/metronome"
echo "Expected: metronome starts running again at 200 bpm in 5/4 time, which is the last setting; metronome resmgr is still running as a process: pidin | grep metronome"
echo start > /dev/local/metronome

echo "K\n********************************************"
echo "cat /dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per- interval: 0.15, nanoSecs: 150000000]"
cat /dev/local/metronome

echo "L\n********************************************"
echo "echo stop > /dev/local/metronome"
echo "Expected: metronome stops running; metronome resmgr is still running as a process: pidin | grep "
metronome
echo stop > /dev/local/metronome

echo "M\n********************************************"
echo "echo stop > /dev/local/metronome"
echo "Expected: metronome remains stopped; metronome resmgr is still running as a process: pidin | grep metronome"
echo stop > /dev/local/metronome

echo "N\n********************************************"
echo "echo start > /dev/local/metronome"
echo "Expected: metronome starts running again at 200 bpm in 5/4 time, which is the last setting; metronome resmgr is still running as a process: pidin | grep metronome"
echo start > /dev/local/metronome

echo "O\n********************************************"
echo "ech start > /dev/local/metronome"
echo "Expected: metronome is still running again at 200 bpm in 5/4 time, which is the last setting; metronome resmgr is still running as a process: pidin | grep metronome"
echo start > /dev/local/metronome

echo "P\n********************************************"
echo "cat /dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per- interval: 0.15, nanoSecs: 150000000]"
cat /dev/local/metronome

echo "Q\n********************************************"
echo "echo pause 3 > /dev/local/metronome"
echo "Expected: metronome continues on next beat (not next measure)."
echo pause 3 > /dev/local/metronome

echo "R\n********************************************"
echo "pause 10 > /dev/local/metronome"
echo "Expected: metronome continues on next beat (not next measure)"
echo pause 10 > /dev/local/metronome

echo "S\n********************************************"
echo "bogus > /dev/local/metronome"
echo "Expected: properly formatted error message, and metronome continues to run."
echo bogus > /dev/local/metronome

echo "T\n********************************************"
echo "echo set 120 2 4 > /dev/local/metronome"
echo "Expected: properly formatted error message, and metronome continues to run"
echo set 120 2 4 > /dev/local/metronome

echo "U\n********************************************"
echo "cat /dev/local/metronome"
echo "Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
cat /dev/local/metronome

echo "V\n********************************************"
echo "cat /dev/local/metronome-help"
echo "Expected: information regarding the metronome resmgr’s API, as seen above"
cat /dev/local/metronome-help

echo "W\n********************************************" 
echo "Writes-Not-Allowed > /dev/local/metronome-help"
echo "Expected: properly formatted error message, and metronome continues to run."
echo "Writes-Not-Allowed > /dev/local/metronome-help"

echo "X\n********************************************"
echo "quit > /dev/local/metronome && pidin | grep metronome"
echo "Expected: metronome gracefully terminates"
echo quit > /dev/local/metronome && pidin | grep metronome


## end of unit tests
exit 0
