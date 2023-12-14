#!/bin/bash
#
#	Run on MacOS
#

difflog() {
	echo "ilogger cancelled by user. check logs..."
	diff  tmmb41.log tmmb42.log
	echo "result: " $?
}

trap "difflog" EXIT

rm tmmb42.log
sleep 1
./ilogger.app <tmmb41.log >tmmb42.log --ownaction highlight --ioformat binary
