# CPP Flages
CPP = g++
#make int as int 4 bytes for faster computation 
CFLAGS = -march=native -mtune=native -flto -O3 -DNDEBUG -funroll-loops -ffast-math -finline-functions -fomit-frame-pointer -pipe -fopenmp

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
	mkdir input -p
	./$(EGEN)
	mkdir output -p

mst: $(EXE) $(EGEN)
	mkdir input -p
	./$(EXE)
	mkdir output -p


# Clean up generated files
clean:
	rm -f $(EXE) $(INPUT2) $(OUTPUT) $(INPUT)

cleanall: clean
	rm -f $(EGEN)
	rm -f input/*
	rm -f output/*
	rmdir input output
