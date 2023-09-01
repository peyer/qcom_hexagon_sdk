PROGRAM = iir

EXEC = $(PROGRAM).exe
OBJS = $(PROGRAM).o
SRCS = ../asm_src/flt_iir_cas_bq.S ../src/iir_cas_bq_c.c test_iir_cas_bq.c 
Q6ARCH = -mv60
CC = hexagon-clang $(Q6ARCH) -O2 -I../include 

all: clean $(EXEC) sim

$(EXEC): $(SRCS)
	$(CC) -o $(EXEC) $(SRCS) -lhexagon


size: 
	$(CC) -DKERNEL_SIZE -c ./src/$(PROGRAM).S
	hexagon-size $(OBJS)



clean:;	rm -f $(OBJS) $(EXEC) stats.txt pmu*.txt

sim:	$(EXEC)
	hexagon-sim $(Q6ARCH) $(EXEC)


