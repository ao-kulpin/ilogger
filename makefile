windows:
	g++  -m64 -D__WINDOWS__ ilogger.cpp sender.cpp option.cpp -o ilogger

linux:
	g++  -m64 -D__LINUX__ ilogger.cpp sender.cpp option.cpp -o ilogger
