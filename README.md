
# MST using new algorithm
Conflict Counting


An implementation of a new minimum spanning tree algorithm.\
Both multithreaded (spin lock barrier + atomics) and single threaded.



To use **UI** 

```bash
  make
```
## Generating Graph
To generate a **matrix / list** of the graph 

```bash
  make  random / list
```
To convert matrix to MY format, keep matrix.txt in main folder

```bash
  make matrix
```

## Generating MST

To get the mst using the paramaters in input_params.json
```bash
  make mst
```
To run the same in a sequential code
```bash
  make single
```
To use Prim's Algo
```bash
  make prim
```
To use **cuda** on Boruvka's Algo
```bash
  make cuda
```

To clean / cleanall
```bash
  make clean / cleanall
```


