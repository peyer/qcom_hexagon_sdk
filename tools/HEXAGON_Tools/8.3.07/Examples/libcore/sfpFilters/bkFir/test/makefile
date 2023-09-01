PROGRAM = bkfir

EXEC = $(PROGRAM).exe
OBJS = $(PROGRAM).o
SRCS = ../asm_src/flt_bkfir.S ../src/bkfir_c.c test_bkfir.c 
Q6VERSION = -mv60
CC = hexagon-clang $(Q6VERSION) -O2 -I../include 

all: clean $(EXEC) sim

$(EXEC): $(SRCS)
	$(CC) -o $(EXEC) $(SRCS) -lhexagon


size: 
	$(CC) -DKERNEL_SIZE -c ./src/$(PROGRAM).S
	hexagon-size $(OBJS)



clean:;	rm -f $(OBJS) $(EXEC) stats.txt pmu*.txt

sim:	$(EXEC)
	hexagon-sim $(Q6VERSION) $(EXEC)


