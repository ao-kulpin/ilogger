#!/bin/bash
#
#	Run on MacOS
#

difflog() {
	echo "ilogger cancelled by user. check logs..."
	diff tmk31.log tmk32.log
	echo "result: " $?
}

trap "difflog" EXIT

rm tmk32.log
sleep 1
./ilogger.app < tmk31.log > tmk32.log --ownaction highlight
