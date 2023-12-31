#!/bin/bash
#
#	Run on Linux with DISABLED Wayland
#

difflog() {
	echo "ilogger cancelled by user. check logs..."
	diff  tlmb41.log tlmb42.log
	echo "result: " $?
}

trap "difflog" EXIT

rm tlmb42.log
./ilogger <tlmb41.log >tlmb42.log --ownaction highlight --ioformat binary
