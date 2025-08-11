# CPP Flages
CPP = g++
CFLAGS = -Wall -Wextra -march=native -flto  -O2

# Files
EXE = mst
EGEN = gen
MST = mst.cpp
GEN = gen.cpp
PYT = visual.py
INPUT = input_params.txt
INPUT2 = input_graph.txt
OUTPUT = output_mst


.PHONY: all matrix mstclean

#Have python3

all:
	python3 $(PYT) &

matrix:
#	$(CPP) $(CFLAGS) -o $(EGEN) $(GEN)
#	./$(EGEN)
#	@echo "Matrix generated successfully."
	@echo "USE cpp to generate matrix"

mst:
#	$(CPP) $(CFLAGS) -o $(EXE) $(MST)
#	./$(EXE) $(INPUT) $(INPUT2) $(OUTPUT)
#	@echo "MST generated successfully."
	@echo "USE cpp to generate MST"
	
# Clean up generated files
clean:
	rm -f $(EXE) $(EGEN) $(INPUT2) $(OUTPUT) $(INPUT)