#!/bin/bash

difflog() {
	echo "ilogger cancelled by user. check logs..."
	if [ ! -s tlk62.log ]; then
		echo "test pass" 
	else
		echo "test fail"
	fi		
}

trap "difflog" EXIT

rm tlk62.log
./ilogger.app < tlk61.log > tlk62.log --ownaction skip 
