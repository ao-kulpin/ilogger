#
# Run on Windows
#

timeout /t 1 && ilogger.exe < tlog1.log > tlog2.log
fc tlog1.log tlog2.log