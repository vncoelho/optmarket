all: test
	

CPPFLAGS = -stdlib=libc++ 
LDFLAGS = -lcpprest -lssl -lcrypto -lboost_system -lboost_thread-mt -lboost_chrono-mt
#-ljsoncpp 
#-Wall 
test:
	g++  -Ofast -std=c++11 main.cpp -ljsoncpp -lpthread -ldl -lcurl -lcrypto -o app_OptMarket

#	~/Downloads/emscriptenNew/emscripten/emcc -O3 -std=c++11 main.cpp -ljsoncpp -lpthread -ldl -lcurl -lcrypto -I/usr/include/x86_64-linux-gnu/ -I/usr/src/linux-headers-4.9.0-deepin13-common/include/linux/ -I/usr/src/linux-headers-4.9.0-deepin13-common/include/ -I/usr/include/ -o 
#~/Downloads/emscriptenNew/emscripten/emcc
	#g++ $(GCC_FLAGS) $(RESTSDK_FGLAS) ./main.cpp -o ./testAquiVamos
	#g++  -std=c++11 main.cpp -o my_file -lcpprest -lssl -lcrypto -lboost_system
	#g++ main.cpp -ljsoncpp -lcurl -ljsonrpccpp-common -ljsonrpccpp-client -o sampleclient


clean:
	#make clean -C ./MyProjects/
	
