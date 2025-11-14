# Makefile

# Default target
all: run_python build_cpp

# Run Python scripts
run_python:
# 	@echo "Running graph_both.py..."
	python graph_both.py
# 	@echo "Running graph_details.py..."
# 	python graph_details.py

# Compile C++ program
build_cpp:
# 	@echo "Compiling main_single.cpp..."
	g++ -std=c++11 -O3 -o mst_single_matrix main_single.cpp
	./mst_single_matrix
# 	@echo "Compiling main_single_list.cpp..."
	g++ -std=c++11 -O3 -o mst_single_list main_single_list.cpp
	./mst_single_list
# 	@echo "Compiling main.cpp..."
	g++ -std=c++11 -pthread -O3 -o mst_matrix main.cpp
	./mst_matrix
# 	@echo "Compiling main_list.cpp..."
	g++ -std=c++11 -pthread -O3 -o mst_list main_list.cpp
	./mst_list 
	g++ -std=c++11 -O3 -o mst_boruvska main_boruvska.cpp
	./mst_boruvska input.txt 

# Clean generated files
clean:
# 	@echo "Cleaning..."
	rm -f mst_list input_list.txt log_list.txt output_list.txt input.txt log.txt output.txt
	rm -f mst_matrix log_single.txt output_single.txt mst_single_matrix mst_single_list log_single_list.txt output_single_list.txt
	rm -f mst_boruvska input.txt mst_boruvska mst_output.txt mst_log.txt a.out