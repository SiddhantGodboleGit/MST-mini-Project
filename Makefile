# CPP Flages
CPP = g++
CFLAGS = -Wall -Wextra -march=native -flto -O2

# Files
EXE = mst.out
EGEN = gen.out
MST = mst.cpp
GEN = gen.cpp
PYT = visual.py
INPUT = input_params.txt
INPUT2 = input_graph.txt
OUTPUT = output_mst


.PHONY: all graph mst clean new cpp

#Have python3

all: $(EXE) $(EGEN)
	python3 $(PYT)

cpp: $(EXE) $(EGEN)

$(EXE): $(MST)
	$(CPP) $(CFLAGS) -o $@ $<

$(EGEN): $(GEN)
	$(CPP) $(CFLAGS) -o $@ $<

graph: $(EXE) $(EGEN)
	mkdir input 
	./$(EGEN)

mst: $(EXE) $(EGEN)
	./$(EXE)

# Clean up generated files
clean:
	rm -f $(EXE) $(INPUT2) $(OUTPUT) $(INPUT)

cleanall: clean
	rm -f $(EGEN)
	rm -f input/*
