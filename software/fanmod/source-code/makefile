nauty_dir=nauty

gtools.o: $(nauty_dir)/gtools.c
	gcc -c -O3 $(nauty_dir)/gtools.c

nautil.o: $(nauty_dir)/nautil.c
	gcc -c -O3 $(nauty_dir)/nautil.c

nauty.o: $(nauty_dir)/nauty.c
	gcc -c -O3 $(nauty_dir)/nauty.c

naugraph.o: $(nauty_dir)/naugraph.c
	gcc -c -O3 $(nauty_dir)/naugraph.c

nautinv.o: $(nauty_dir)/nautinv.c
	gcc -c -O3 $(nauty_dir)/nautinv.c

graph64.o: graph64.cpp
	g++ -c graph64.cpp

output.o: output.cpp
	g++ -c output.cpp

random.o: random.cpp
	g++ -c random.cpp

maingraph.o: maingraph.cpp
	g++ -c maingraph.cpp

main.o: main.cpp
	g++ -c main.cpp

all: graph64.o output.o random.o maingraph.o main.o gtools.o nautil.o nauty.o naugraph.o nautinv.o
	g++ -o fanmod_linux graph64.o output.o random.o maingraph.o main.o gtools.o nautil.o nauty.o naugraph.o nautinv.o

clean:
	rm -f fanmod_linux *.o

