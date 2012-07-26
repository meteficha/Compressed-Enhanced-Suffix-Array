CC= gcc
CFLAGS = -Wall  -g
CFLAGSFINAL = -Wall -O2
CFLAGSPROFILE = -Wall -pg
LDFLAGS = -lm
LDFLAGSTEST = -lm -lcheck
SRC = alloc.c bit.c compression.c csa.c dataStructures.c suffixSort.c text.c utils.c QSufSort.c debug.c search.c
SRCTST = alloc.c bit.c compression.c csa.c dataStructures.c suffixSort.c text.c utils.c QSufSort.c testBit.c debug.c search.c
OBJ = $(SRC:.c=.o)
OBJTST = $(SRCTST:.c=.o)



all: final generator

objDebug: $(SRC)
	@echo "\n\nBuilding debug Index sources\n\n"
	gcc -c $(CFLAGS) $(SRC) 

objFinal: $(SRC)
	@echo "\n\nBuilding optimized Index sources\n\n"
	gcc -c $(CFLAGSFINAL) $(SRC) 
	
objProfile: $(SRC)
	@echo "\n\nBuilding profile Index sources\n\n"
	gcc -c $(CFLAGSPROFILE) $(SRC) 


objTest: $(SRCTST)
	@echo "\n\nBuilding debug test sources\n\n"
	gcc -c $(CFLAGS) $(SRCTST) 

profile: objProfile
	@echo "\n\nBuilding profile Index sources\n\n"
	$(CC) $(CFLAGSPROFILE) main.c -o index  $(OBJ) $(LDFLAGS)

debug: objDebug objTest control.c
	@echo "\n\nBuilding debug Index sources\n\n"
	$(CC) $(CFLAGS) main.c -o index  $(OBJ) $(LDFLAGS)
	@echo "\n\nBuilding debug Control sources\n\n"
	gcc -g control.c alloc.o text.o -o control -ldivsufsort
		
final: objFinal objFinal control.c
	@echo "\n\nBuilding Index\n\n"
	$(CC) $(CFLAGSFINAL) main.c -o index  $(OBJ) $(LDFLAGS)
	@echo "Building Control"
	gcc $(CFLAGSFINAL) control.c alloc.o text.o -o control -ldivsufsort 
	
generator: textGenerator.c
	@echo "\n\nBuilding Generator\n\n"
	gcc $(CFLAGSFINAL) textGenerator.c -o textGenerator
	
test: objTest
	@echo "\n\nBuilding Test\n\n"
	$(CC) $(CFLAGS) test.c  $(OBJTST) -o teste $(LDFLAGSTEST)

clean:
	rm -rf $(OBJ) $(OBJTST) index control textGenerator test
