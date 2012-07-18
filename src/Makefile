CC=g++-4.7
CFLAGS=-Wall -std=c++11 -g -rdynamic
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=dstore.o
SOURCES=DStore.cc Dictionary.cc Element.cc main.cc

all: clean dstore

dstore:
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(INCLUDE) $(SOURCES)

clean:
	rm *.o
