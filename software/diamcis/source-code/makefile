accumulatore.o: accumulatore.cxx
	g++ -c -pg accumulatore.cxx

coniugato.o: coniugato.cxx
	g++ -c -pg coniugato.cxx
	
mappatura3.o: mappatura3.cxx
	g++ -c -pg mappatura3.cxx
	
scambia.o: scambia.cxx
	g++ -c -pg scambia.cxx
	
quick.o: quick.cxx
	g++ -c -pg quick.cxx
	
ordina.o: ordina.cxx
	g++ -c -pg ordina.cxx	

ordinabool.o: ordinabool.cxx
	g++ -c -pg ordinabool.cxx
	
quickbool.o: quickbool.cxx
	g++ -c -pg quickbool.cxx

scambiabool.o: scambiabool.cxx
	g++ -c -pg scambiabool.cxx

Rbis.o: Rbis.cxx
	g++ -c -pg Rbis.cxx									
	
all: accumulatore.o mappatura3.o coniugato.o scambia.o quick.o ordina.o ordinabool.o quickbool.o scambiabool.o Rbis.o
	g++ -o diamcis_linux -pg accumulatore.o mappatura3.o coniugato.o scambia.o quick.o ordina.o ordinabool.o quickbool.o scambiabool.o Rbis.o
	
clean:
	rm *.o diamcis_linux
