all: test
	

CPPFLAGS = -stdlib=libc++ 
LDFLAGS = -lcpprest -lssl -lcrypto -lboost_system -lboost_thread-mt -lboost_chrono-mt
#-ljsoncpp 
#-Wall 
test:
	#g++ $(GCC_FLAGS) $(RESTSDK_FGLAS) ./main.cpp -o ./testAquiVamos
	#g++  -std=c++11 main.cpp -o my_file -lcpprest -lssl -lcrypto -lboost_system
	g++ -std=c++11 main.cpp -ljsoncpp -lpthread -ldl -lcurl -lcrypto -o my_file 
	#g++ main.cpp -ljsoncpp -lcurl -ljsonrpccpp-common -ljsonrpccpp-client -o sampleclient


clean:
	#make clean -C ./MyProjects/
	