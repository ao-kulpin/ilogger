#!/bin/bash

difflog() {
	echo "ilogger cancelled by user. check logs..."
	diff tlk21.log tlk22.log
	echo "result: " $?
}

trap "difflog" EXIT

rm tlk22.log
./ilogger < tlk21.log > tlk22.log
