#!/bin/bash
#
#	Run on Linux with DISABLED Wayland
#

difflog() {
	echo "ilogger cancelled by user. check logs..."
	diff tlm11.log tlm12.log
	echo "result: " $?
}

trap "difflog" EXIT

rm tlm12.log
./ilogger < tlm11.log > tlm12.log
