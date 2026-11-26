PP3: main.o Simulator.o
	g++ main.o Simulator.o -o PP3

main.o : main.cpp Simulator.h
	g++ -c main.cpp

Simulator.o : Simulator.cpp Simulator.h
	g++ -c Simulator.cpp

clean: 
	rm *.o PP3