windows:
	g++  -D__WINDOWS__ ilogger.cpp sender.cpp option.cpp -o ilogger

linux:
#	g++  -D__LINUX__  ilogger.cpp sender.cpp option.cpp -o ilogger -lX11 -lXt -lX11  -lXext -lXi -lXtst
	g++  -D__LINUX__  ilogger.cpp sender.cpp option.cpp -o ilogger -lX11 -lXtst
