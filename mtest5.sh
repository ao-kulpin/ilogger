#!/bin/bash
#
#	Run on MacOS
#

difflog() {
	echo "ilogger cancelled by user. check logs..."
	diff  tmkb51.log tmkb52.log
	echo "result: " $?
}

trap "difflog" EXIT

rm tmkb52.log
sleep 1
./ilogger.app <tmkb51.log >tmkb52.log --ownaction highlight --ioformat binary
