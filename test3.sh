#!/bin/bash
#
#	Run on Linux with DISABLED Wayland
#

difflog() {
	echo "ilogger cancelled by user. check logs..."
	diff tlk31.log tlk32.log
	echo "result: " $?
}

trap "difflog" EXIT

rm tlk32.log
./ilogger < tlk31.log > tlk32.log --ownaction highlight
