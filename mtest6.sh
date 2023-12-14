#!/bin/bash
#
#	Run on MacOS
#

difflog() {
	echo "ilogger cancelled by user. check logs..."
	if [ ! -s tmmb62.log ]; then
		echo "test pass" 
	else
		echo "test fail"
	fi		
}

trap "difflog" EXIT

rm tmmb62.log
sleep 1
./ilogger.app < tmmb61.log > tmmb62.log --ownaction skip --ioformat binary
