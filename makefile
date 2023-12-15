windows:
	g++ -std=c++11 -D__WINDOWS__ ilogger.cpp sender.cpp option.cpp -o ilogger
	@echo ****************************
	@echo Build Succeeded: ilogger.exe
	@echo ****************************

linux:
#	g++  -D__LINUX__  ilogger.cpp sender.cpp option.cpp -o ilogger -lX11 -lXt -lX11  -lXext -lXi -lXtst
	g++  -D__LINUX__  ilogger.cpp sender.cpp option.cpp -o ilogger -lX11 -lXtst

macos:
	g++  -std=c++11 -framework ApplicationServices -D__MACOS__  ilogger.cpp sender.cpp option.cpp -o ilogger.app
	@echo 
	@echo 'Build Succeeded: ilogger.app'
	@echo '****************************'
