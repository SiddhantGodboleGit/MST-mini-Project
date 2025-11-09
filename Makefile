# CPP Flages
CPP = g++
#make int as int 4 bytes for faster computation 
CFLAGS = -march=native -O3 -flto -ffast-math


# Files
EXE = mst.out
EGEN = gen.out
MST = mst.cpp
GEN = gen.cpp
PYT = visual.py
INPUT = input_params.txt
INPUT2 = input_graph.txt
OUTPUT = output_mst


.PHONY: all graph mst clean new cpp matrix single random

#Have python3

all: $(EXE) $(EGEN)
	python3 $(PYT)

cpp: $(EXE) $(EGEN)

$(EXE): $(MST)
	$(CPP) $(CFLAGS) -o $@ $<

$(EGEN): $(GEN)
	$(CPP) $(CFLAGS) -o $@ $<

graph: $(EXE) $(EGEN)
	mkdir input -p
	mkdir output -p
	./$(EGEN)

mst: $(EXE) $(EGEN)
	mkdir input -p
	mkdir output -p
	./$(EXE)

matrix: matrix_to_mine.cpp
	mkdir input -p
	$(CPP) $(CFLAGS) -o matrix_to_mine.out matrix_to_mine.cpp
	./matrix_to_mine.out

single: mst_single.cpp
	mkdir input -p
	mkdir output -p
	$(CPP) $(CFLAGS) -o mst_single.out mst_single.cpp
	./mst_single.out

random: $(EXE) $(EGEN)
	mkdir input -p
	mkdir output -p
	./$(EGEN) random


# Clean up generated files
clean:
	rm -f $(EXE) $(INPUT2) $(OUTPUT) $(INPUT) matrix_to_mine.out mst_single.out

cleanall: clean
	rm -f $(EGEN)
	rm -f input/*
	rm -f output/*
	rmdir input output
