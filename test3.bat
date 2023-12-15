#
# Run on Windows
#

timeout /t 1 && ilogger --ioformat binary <tbl31.log >tbl32.log
fc /b tbl31.log tbl32.log