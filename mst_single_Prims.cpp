// SINGLE THREADED version of my new algo for mst

// Name Conflict resolution


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <queue>
#include <utility>
#include <limits>
using namespace std;
#define metyr 1

int ** graph;
int * last_index;
int * sizess;
int * minl;
int * minv;
int * meta;
int * my_grp;
int * groupn;
int * leaves;
int * branch_minl;
int * branch_minv;
int * weights;
int * order1;
int * order2;
bool condition_1 = 0;
bool condition_2 = 0;

long nodes = 64;
long long edges = 256;
int threads = 4;
uint node_num = 0;
uint total_weight = 0;
uint edge = 0;
uint done = 0;
uint order = 0;
uint conflict = 0;
uint leaf_count = 0;
string filename_complete;

void mst_to_file(int * MST1, int * MST2){
    string filename = filename_complete;
    filename = filename.substr(0, filename.find_last_of('.')) + "_mst.txt";
    //replace input with output
    filename = "output/" + filename.substr(filename.find_last_of('/') + 1);
    ofstream outfile;
    outfile.open(filename , std::ios::out | std::ios::trunc);
    int total = edge;
    for (int i = 0; i < total; i++){
        int u = MST1[i];
        int v = MST2[i];
        int w = weights[i];
        outfile << u << " " << v << " "  << w << "\n";
    }
    cout << "MST written to file \033[1m" << filename << "\033[0m with total weight " << total_weight << ".\n";
    outfile.close();
    return;
}

void print_graph(){
    cout << "Graph Adjacency Matrix:\n";
    for (int i = 0; i < nodes; i++){
        for (int j = 0; j < sizess[i]; j++){
            cout << graph[i][j] << " ";
        }
        cout << "\n";
    }
}

void print_meta(){
    cout << "minl of MST:\n";
    for (int i = 0; i < nodes; i++){
        cout << minl[i] << " ";
    }
    cout << endl;
    cout << "meta of MST:\n";
    for (int i = 0; i < nodes; i++){
        cout << meta[i] << " ";
    }
    cout << endl;
}

bool get_ready(){
    int  seed;
    int thread_temp = 0;
    bool connected = 0 , complete = 0, regular = 0;
    ifstream infile("input_params.json");
    if (!infile.is_open()) {
        cerr << "Unable to open file  \033[1minput_params.json\033[0m\n";
        return 0;
    }
    while (!infile.eof()) { // read input parameters from json file 
        string key;
        char colon;
        if (infile >> key) {
            if (key == "\"seed\":") infile >> colon >> seed;
            else if (key == "\"nodes\":") infile >> colon  >> nodes;
            else if (key == "\"edges\":") infile >> colon  >> edges;
            else if (key == "\"threads\":") infile >> colon  >> thread_temp;
            else if (key == "\"connected\":"){ infile  >> key; connected = (key == "true" || key == "true,"); }
            else if (key == "\"complete\":") { infile   >> key; complete = (key == "true" || key == "true,"); }
            else if (key == "\"regular\":") { infile  >> key; regular = (key == "true" || key == "true,"); }
        }
    }
    infile.close();

    if (nodes < 2) nodes = 2;
    if (edges > ((nodes)*(nodes-1))/2 ) edges = ((nodes-1)*(nodes))/2;
    if (edges < 0) edges = 0;
    if (thread_temp < 1) thread_temp = 1;
    if (thread_temp > nodes) thread_temp = nodes;
    if (complete) edges = ((nodes-1)*(nodes))/2;
    if (connected && edges < nodes -1) edges = nodes -1;
    if (complete==1 && regular==1){
        regular = 0;
    }
    
    if (regular == 1){
        int max_edges = (nodes*(nodes-1))/2;
        int min_edges = nodes;
        if (edges < min_edges) edges = min_edges;
        if (edges > max_edges) edges = max_edges;
        edges = (edges / nodes) * nodes; // make it multiple of nodes   
    }
    threads = thread_temp;
    srand(seed);    

    if (filename_complete == "")
    filename_complete = "input/" + to_string(nodes) + "_" + to_string(edges) + "_" + to_string(seed) + "_" + to_string(connected) + to_string(complete) + to_string(regular) + ".txt";

    cout << "Reading graph from file \033[1m" << filename_complete << "\033[0m ...\n";
    ifstream testfile;
    testfile.open(filename_complete);
    if (!testfile.is_open()){
        cerr << "File \033[1m" << filename_complete << "\033[0m does not exist. Please generate the graph first using 'make graph'.\n";
        return 0;
    }
    graph = new int*[nodes];                                // allocate space for graph

    // read first line to initialize graph matrix
    // size of each row is edges + () + 1
    string line;

    getline(testfile, line);
    istringstream iss(line);
    int edge_count;
    int * last_entry = new int[nodes]();
    char paren;

    last_index = new int[nodes]();
    sizess = new int[nodes]();
    for (long i = 0; i < nodes; i++){
        last_index[i] = -1;
        last_entry[i] = -1;
        int sizez = 0;
        iss >> edge_count >> paren >> sizez >> paren;
        // cout << "Node " << i << " has " << edge_count << " edges and " << sizez << " zeros.\n";
        sizess[i] = sizez + edge_count;
        graph[i] = new int32_t[sizess[i]+1];             // int 32 for smaller size
        // cout << "Node " << i << " has " << edge_count << " edges and " << sizess[i] << " zeros.\n";
    }


    //read input file to graph matrix
    int i = 0, j = 0, weight = 0;
    while (getline(testfile, line)) {
        istringstream iss(line);
        //parse same way
        while (iss >> j >> paren >> weight >> paren) {
            j = j; // zero based indexing
            if (weight < 0) weight = 1;
            if (last_index[i] + 1 == j){
                graph[i][++last_entry[i]] = weight; 
            }
            else {
                graph[i][++last_entry[i]] = -(j); // mark zeros as negative count
                graph[i][++last_entry[i]] = weight;
                // cout << "Inserted " << (j - last_index[i] - 1) << " zeros for node " << i << " before edge to node " << j << ".\n";
            }

            if (last_index[j] + 1 == i){
                graph[j][++last_entry[j]] = weight; 
            }
            else {
                graph[j][++last_entry[j]] = -(i); // mark zeros as negative count
                graph[j][++last_entry[j]] = weight;
                // cout << "Inserted " << (i - last_index[j] - 1) << " zeros for node " << j << " before edge to node " << i << ".\n";
            }
            last_index[i] = j;
            last_index[j] = i;
        }
        i++;
    }
    // print_graph();
    testfile.close();
    delete[] last_entry;
    return 1;
}

void MST(int * MST1, int * MST2 , int id){
    // Replace complex custom algorithm with a simple Prim's algorithm using a min-heap.

    edge = 0;
    total_weight = 0;

    vector<char> inMST(nodes, 0);
    vector<int> key(nodes, INT32_MAX);
    vector<int> parent(nodes, -1);

    using pii = pair<int,int>;
    priority_queue<pii, vector<pii>, greater<pii>> pq; // (weight, node)

    // start from node 0
    key[0] = 0;
    pq.push({0, 0});

    while (!pq.empty()){
        auto p = pq.top(); pq.pop();
        int u = p.second;
        int w = p.first;
        if (inMST[u]) continue;
        inMST[u] = 1;

        if (parent[u] != -1){
            MST1[edge] = parent[u];
            MST2[edge] = u;
            weights[edge] = w;
            edge++;
            total_weight += w;
            if (edge >= (int)nodes - 1) break;
        }

        // iterate neighbors from the packed `graph[u]` representation
        int idx = 0;
        for (int j = 0; j < sizess[u]; ++j){
            int val = graph[u][j];
            if (val >= 0){
                int v = idx;
                int wt = val;
                if (!inMST[v] && wt < key[v]){
                    key[v] = wt;
                    parent[v] = u;
                    pq.push({wt, v});
                }
                idx++;
            } else {
                idx = -val;
            }
        }
    }
    return;
}

int main(int argc, char** argv){
    if (argc > 1)
        filename_complete = argv[1];

    if (get_ready() == 0){ cout << "Pls fix input\n"; return 1; }    // read input, alllocate space and initialize graph
        // For fairness no preprocessing is done pure graph in matrix
        //start timer

    minl = new int[nodes];       // array to store min locations
    minv = new int[nodes];       // array to store min values
    meta = new int[nodes]();
    my_grp = new int[nodes];
    leaves = new int[nodes];
    order1 = new int[nodes];
    groupn = new int[nodes];
    order2 = new int[nodes];
    branch_minl = new int[nodes];
    branch_minv = new int[nodes]();
    // atomic_bool * inMST = new atomic_bool[nodes]();
    int * MST1 = new int[nodes]();
    int * MST2 = new int[nodes]();
    weights = new int[nodes]();
    node_num = 0;               // reset node counter for threads

    int thread_temp = threads;
    // thread thread_ids[thread_temp];


    cout << nodes << " nodes, " << edges << " edges, " << threads << " threads.\n";
    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

    MST(MST1, MST2 , 0);         // run mst algorithm


    // for (int i = 0; i < thread_temp; i++) {
    //     thread_ids[i] = thread(MST,  MST1, MST2     , i);
    // }

    // for (int i = 0; i < thread_temp; i++) {
    //     thread_ids[i].join();
    // }

    // print_graph();
    // print_meta();
    cout << edge << " edges in MST.\n";
    
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> duration = end - start;
    cout << "Time: \033[1m" << (double)duration.count() / 1000 << "\033[0m seconds" << endl;
    asm volatile("" ::: "memory");
    
    mst_to_file(MST1, MST2);
    // free memory

    // print_graph();
    delete[] MST1;
    delete[] MST2;
    delete[] weights;

    delete[] last_index;
    delete[] sizess;
    delete[] minl;
    delete[] minv;
    delete[] meta;
    delete[] my_grp;
    delete[] groupn;
    delete[] leaves;
    delete[] branch_minl;
    delete[] branch_minv;
    delete[] order1;
    delete[] order2;

        
        for (int i = 0; i < nodes; i++){
            if (graph[i] != nullptr) {
                delete[] graph[i];
                graph[i] = nullptr; // Set to nullptr to avoid double deletion
            }
        }
    delete[] graph;
    return 0;
}