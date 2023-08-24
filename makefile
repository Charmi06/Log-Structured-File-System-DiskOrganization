all: main.o
	g++ -std=c++11	-g main.o -o main

main.o:	main.cpp commands_functions.cpp func.cpp
	g++ -std=c++11 -g -c main.cpp

initialization.o: initialization.cpp
	g++ -std=c++11 -g -c initialization.cpp

DriveLink: initialization.o
	g++ -std=c++11 -g initialization.o -o initialization

drive: DriveLink
	./initialization

clean:
	rm -f *.o *~ *.swp main initialization s b;
	rm -rf SegmentChaeckPointDrive/;

again:	clean all drive
