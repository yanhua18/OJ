LDFLAGS= -L /root/third_part/lib
DEBUGFLAGS=-g
BIN=oj_server
CC=g++

.PHONY:$(BIN)
$(BIN):oj_server.cpp
	$(CC) $^ -o $@ $(DEBUGFLAGS) $(LDFLAGS) -I /root/third_part/include -std=c++11 -lctemplate -ljsoncpp -lpthread

.PHONY:clean
clean:
	rm -f $(BIN)


