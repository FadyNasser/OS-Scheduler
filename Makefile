build: 
	g++ clk.cpp -std=c++11 -o clock.out
	g++ scheduler.cpp -std=c++11 -o sch.out
	g++ FilesGenerator.cpp -std=c++11 -o Files.out
	g++ processGenerator.cpp -std=c++11 -o main.out
	g++ process.cpp -std=c++11 -o pro.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./main.out