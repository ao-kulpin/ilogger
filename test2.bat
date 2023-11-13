timeout /t 1 && ilogger --ioformat binary <tbl1.log >tbl2.log
fc /b tbl1.log tbl2.log