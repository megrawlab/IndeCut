
JFLAGS = -g
JC = javac
JAR = jar
JVM = 1.8
out_jarfile = comofinder_randout.jar

BIN = classes

LIB = lib/jsr166y-1.7.0.jar

JAVAFLAGS = -g -d $(BIN) -cp $(LIB) -target $(JVM)

SRC = src/ccbr/utoronto/ca/*/*.java

manifest_file = MANIFEST.MF

compile:
	$(JC) $(JAVAFLAGS) $(SRC)

makejar: 
	$(JAR) cvfm $(out_jarfile) $(manifest_file) -C $(BIN) .

all: compile makejar
	echo "$(out_jarfile) created!"

clean:
	rm -f -r $(BIN)/*
	rm -f $(out_jarfile)
