# CPP Flages
CPP = g++
#make int as int 4 bytes for faster computation 
CFLAGS = -march=native -O3 -flto -ffast-math


# Files
EXE = mst.out
EGEN = gen.out
MST = mst.cpp
GEN = gen.cpp
PYT = visual/visual.py
INPUT = input_params.txt
INPUT2 = input_graph.txt
OUTPUT = output_mst

W = 64


.PHONY: all graph mst clean new cpp matrix single random list compare

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

list: $(EXE) $(EGEN)
	mkdir input -p
	mkdir output -p
	./$(EGEN) list random

# input will be make compare {number}
compare:
	$(CPP) $(CFLAGS) comparision/compare.cpp -o comparision/compare.out
	$(CPP) $(CFLAGS) comparision/MainFilterKruskal.cpp -o comparision/MainFilterKruskal.out
	$(CPP) $(CFLAGS) comparision/MainMST.cpp -o comparision/MainMST.out
	./comparision/MainFilterKruskal.out comparision/list.txt ${W}
	./comparision/MainMST.out comparision/list.txt ${W}
	./comparision/compare.out

# Clean up generated files
clean:
	rm -f $(EXE) $(INPUT2) $(OUTPUT) $(INPUT) matrix_to_mine.out mst_single.out

cleanall: clean
	rm -f $(EGEN)
	rm -f input/*
	rm -f output/*
	rm -f matrix.txt
	rm -f comparision/list.txt
	rmdir input output
	rm -f comparision/compare.out comparision/MainFilterKruskal.out comparision/MainMST.out list.txt
