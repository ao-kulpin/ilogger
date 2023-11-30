#!/bin/bash

difflog() {
	echo "ilogger cancelled by user. check logs..."
	diff  tlkb51.log tlkb52.log
	echo "result: " $?
}

trap "difflog" EXIT

rm tlkb52.log
./ilogger <tlkb51.log >tlkb52.log --ownaction highlight --ioformat binary
