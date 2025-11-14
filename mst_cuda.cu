#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <chrono>

// Simple CUDA Boruvka-style MST based on edge list built from existing adjacency matrix format

#define CUDA_CHECK(x) do { cudaError_t err = (x); if (err != cudaSuccess) { \
    fprintf(stderr, "CUDA Error %s:%d: %s\n", __FILE__, __LINE__, cudaGetErrorString(err)); exit(1);} } while(0)

static long nodes = 0;
static long long edges_input = 0;
std::string filename_complete;

int **graph = nullptr;    // host adjacency representation (same as mst.cpp)
int *sizess = nullptr;     


int *d_edgesU = nullptr; 
int *d_edgesV = nullptr;  
int *d_edgesW = nullptr;  


int *d_parent = nullptr;   // length N

unsigned long long *d_bestPacked = nullptr; // length N

// Host edge list
std::vector<int> edgesU, edgesV, edgesW; // unique edges u<v

// Host MST result
std::vector<int> MSTU, MSTV, MSTW;

// Kernel: initialize bestPacked to UINT64_MAX
__global__ void initBestKernel(unsigned long long *best, int n) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid < n) best[tid] = ULLONG_MAX; // indicates no edge yet
}

// Device find with limited path compression (single step)
__device__ int d_find(int x, int *parent){
    while (parent[x] != x){
        parent[x] = parent[parent[x]]; // partial compression
        x = parent[x];
    }
    return x;
}

__global__ void bestEdgeKernel(const int *edgesU, const int *edgesV, const int *edgesW, int E, int *parent, unsigned long long *best){
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= E) return;
    int u = edgesU[tid];
    int v = edgesV[tid];
    int w = edgesW[tid];
    int pu = d_find(u, parent);
    int pv = d_find(v, parent);
    if (pu == pv) return; // internal edge
    unsigned long long packed = ( (unsigned long long)w << 32 ) | (unsigned long long)tid;
    // update component pu
    atomicMin(&best[pu], packed);
    atomicMin(&best[pv], packed);
}

// Host utility: read input like mst.cpp but only need graph matrix
bool read_input(){
    int seed; int threads_dummy; bool connected=0, complete=0, regular=0; long long edges_param=0; long nodes_param=0;
    std::ifstream infile("input_params.json");
    if (!infile.is_open()){ std::cerr << "Unable to open input_params.json\n"; return false; }
    while (!infile.eof()){
        std::string key; char colon; if (infile >> key){
            if (key == "\"seed\":") infile >> colon >> seed;
            else if (key == "\"nodes\":") infile >> colon >> nodes_param;
            else if (key == "\"edges\":") infile >> colon >> edges_param;
            else if (key == "\"threads\":") infile >> colon >> threads_dummy;
            else if (key == "\"connected\":") { infile >> key; connected = (key=="true" || key=="true,"); }
            else if (key == "\"complete\":") { infile >> key; complete = (key=="true" || key=="true,"); }
            else if (key == "\"regular\":") { infile >> key; regular = (key=="true" || key=="true,"); }
        }
    }
    infile.close();
    nodes = nodes_param; edges_input = edges_param; if (nodes < 2) nodes = 2;
    if (filename_complete.empty()){
        filename_complete = "input/" + std::to_string(nodes) + "_" + std::to_string(edges_input) + "_" + std::to_string(seed) + "_" + std::to_string(connected) + std::to_string(complete) + std::to_string(regular) + ".txt";
    }
    std::ifstream testfile(filename_complete);
    if (!testfile.is_open()){ std::cerr << "Graph file " << filename_complete << " missing. Run 'make graph'.\n"; return false; }

    std::string line; getline(testfile, line); std::istringstream iss(line); int edge_count; char paren;
    sizess = new int[nodes];
    graph = new int*[nodes];
    int * last_index = new int[nodes]();
    int * last_entry = new int[nodes]();
    for (long i=0;i<nodes;i++){ last_index[i]=-1; last_entry[i]=-1; int sizez=0; iss >> edge_count >> paren >> sizez >> paren; sizess[i]= sizez + edge_count; graph[i] = new int32_t[sizess[i]+1]; }
    int i=0,j=0,w=0; while (getline(testfile,line)){
        std::istringstream is2(line);
        while (is2 >> j >> paren >> w >> paren){ if (w<0) w=1; if (last_index[i]+1==j){ graph[i][++last_entry[i]] = w; } else { graph[i][++last_entry[i]] = -j; graph[i][++last_entry[i]] = w; }
            if (last_index[j]+1==i){ graph[j][++last_entry[j]] = w; } else { graph[j][++last_entry[j]] = -i; graph[j][++last_entry[j]] = w; }
            last_index[i]=j; last_index[j]=i; }
        i++; }
    delete[] last_index; delete[] last_entry; testfile.close();

    // Build unique edge list (u < v)
    for (int u=0; u<nodes; ++u){
        int idx=0; for (int k=0; k<sizess[u]; ++k){ int val = graph[u][k]; if (val>=0){ int v = idx; int wgh = val; if (v>u){ edgesU.push_back(u); edgesV.push_back(v); edgesW.push_back(wgh);} idx++; } else { idx = -val; } }
    }
    std::cout << "Read graph with " << nodes << " nodes and built edge list of " << edgesU.size() << " unique edges.\n";
    return true;
}

// Boruvka iteration on GPU selecting best edges; unions on host.
void gpu_boruvka(){
    int N = nodes; int E = (int)edgesU.size();
    // Allocate device memory
    CUDA_CHECK(cudaMalloc(&d_edgesU, E * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_edgesV, E * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_edgesW, E * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_parent, N * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_bestPacked, N * sizeof(unsigned long long)));

    CUDA_CHECK(cudaMemcpy(d_edgesU, edgesU.data(), E*sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_edgesV, edgesV.data(), E*sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_edgesW, edgesW.data(), E*sizeof(int), cudaMemcpyHostToDevice));

    // Host parent
    std::vector<int> parent(N); for (int i=0;i<N;++i) parent[i]=i;

    auto find_host = [&](int x){ while (parent[x]!=x){ parent[x]=parent[parent[x]]; x=parent[x]; } return x; };
    auto unite_host = [&](int a, int b){ a=find_host(a); b=find_host(b); if (a==b) return false; if (a<b) parent[b]=a; else parent[a]=b; return true; };

    int components = N;
    MSTU.reserve(N-1); MSTV.reserve(N-1); MSTW.reserve(N-1);

    int block = 256; int gridEdges = (E + block -1)/block; int gridNodes = (N + block -1)/block;

    while (components > 1){
        // Copy parent to device
        CUDA_CHECK(cudaMemcpy(d_parent, parent.data(), N*sizeof(int), cudaMemcpyHostToDevice));
        // Init best
        initBestKernel<<<gridNodes, block>>>(d_bestPacked, N);
        CUDA_CHECK(cudaDeviceSynchronize());
        // Find best edges across components
        bestEdgeKernel<<<gridEdges, block>>>(d_edgesU, d_edgesV, d_edgesW, E, d_parent, d_bestPacked);
        CUDA_CHECK(cudaDeviceSynchronize());
        // Copy bestPacked back
        std::vector<unsigned long long> bestPacked(N);
        CUDA_CHECK(cudaMemcpy(bestPacked.data(), d_bestPacked, N*sizeof(unsigned long long), cudaMemcpyDeviceToHost));
        // Apply unions
        bool any = false;
        for (int c=0;c<N;++c){ if (parent[c]!=c) continue; unsigned long long packed = bestPacked[c]; if (packed==ULLONG_MAX) continue; int w = (int)(packed >> 32); int eIdx = (int)(packed & 0xffffffffULL); int u = edgesU[eIdx]; int v = edgesV[eIdx]; int pu = find_host(u); int pv = find_host(v); if (pu==pv) continue; if (unite_host(pu,pv)){ components--; any=true; MSTU.push_back(u); MSTV.push_back(v); MSTW.push_back(w);} }
        if (!any){ // graph may be disconnected
            std::cerr << "No connecting edges found. Graph may be disconnected. Remaining components: " << components << "\n"; break; }
    }
    CUDA_CHECK(cudaFree(d_edgesU)); CUDA_CHECK(cudaFree(d_edgesV)); CUDA_CHECK(cudaFree(d_edgesW)); CUDA_CHECK(cudaFree(d_parent)); CUDA_CHECK(cudaFree(d_bestPacked));
}

void mst_to_file(){
    std::string filename = filename_complete; filename = filename.substr(0, filename.find_last_of('.')) + "_mst.txt"; filename = "output/" + filename.substr(filename.find_last_of('/') + 1);
    std::ofstream outfile(filename, std::ios::out | std::ios::trunc);
    long long total_weight = 0; for (size_t i=0;i<MSTU.size();++i){ outfile << MSTU[i] << " " << MSTV[i] << " " << MSTW[i] << "\n"; total_weight += MSTW[i]; }
    outfile.close();
    std::cout << "MST written to " << filename << " with " << MSTU.size() << " edges.\n";
}

int main(int argc, char** argv){
    if (argc > 1) filename_complete = argv[1];
    if (!read_input()) return 1;
    std::cout << "Launching CUDA Boruvka MST...\n";
    auto start = std::chrono::high_resolution_clock::now();
    gpu_boruvka();
    auto end = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(end-start).count();
    std::cout << "GPU MST time: " << secs << " s\n";
    mst_to_file();
    // cleanup host adjacency
    if (graph){ for (int i=0;i<nodes;++i){ delete[] graph[i]; } delete[] graph; }
    delete[] sizess;
    return 0;
}
