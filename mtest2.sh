#!/bin/bash
#
#	Run MacOS
#

difflog() {
	echo "ilogger cancelled by user. check logs..."
	diff tmk21.log tmk22.log
	echo "result: " $?
}

trap "difflog" EXIT

rm tmk22.log
sleep 1
./ilogger.app < tmk21.log > tmk22.log
