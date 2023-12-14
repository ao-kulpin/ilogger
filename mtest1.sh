#!/bin/bash
#
#	Run on MacOS
#

difflog() {
	echo "ilogger cancelled by user. check logs..."
	diff tmm11.log tmm12.log
	echo "result: " $?
}

trap "difflog" EXIT

rm tmm12.log
sleep 1
./ilogger.app < tmm11.log > tmm12.log
