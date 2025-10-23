#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <condition_variable>
using namespace std;
#define metyr 2

int ** graph;
int * mins;
int * meta;
int * groupn;
mutex mstx;
bool condition_1 = 0;
bool condition_2 = 0;
condition_variable cv;

int nodes = 64, edges = 256, threads = 4;
atomic_uint node_num(0);
atomic_uint groups(0);
atomic_uint edge(0);
atomic_uint done(0);
atomic_uint conflict(0);
string filename_complete;

void print_graph(){
    cout << "Graph Adjacency Matrix:\n";
    for (int i = 0; i < nodes; i++){
        for (int j = 0; j < nodes; j++){
            cout << graph[i][j] << " ";
        }
        cout << "\n";
    }
}

void print_meta(){
    int total_weight = 0;
    cout << "meta of MST:\n";
    for (int i = 0; i < nodes; i++){
        cout << meta[i] << " ";
    }
    cout << endl;
}

void get_ready(){
    int  seed, nodes, edges;
    bool connected = 0 , complete = 0, regular = 0;
    ifstream infile("input_params.json");
    if (!infile.is_open()) {
        cerr << "Unable to open file  \033[1minput_params.json\033[0m\n";
        exit(1);
    }
    while (!infile.eof()) { // read input parameters from json file 
        string key;
        char colon;
        if (infile >> key) {
            if (key == "\"seed\":") infile >> colon >> seed;
            else if (key == "\"nodes\":") infile >> colon  >> nodes;
            else if (key == "\"edges\":") infile >> colon  >> edges;
            else if (key == "\"threads\":") infile >> colon  >> threads;
            else if (key == "\"connected\":"){ infile >> colon  >> key; connected = (key == "true" || key == "true,"); }
            else if (key == "\"complete\":") { infile >> colon  >> key; complete = (key == "true" || key == "true,"); }
            else if (key == "\"regular\":") { infile >> colon  >> key; regular = (key == "true" || key == "true,"); }
        }
    }
    infile.close();

    if (nodes < 2) nodes = 2;
    if (edges > ((nodes)*(nodes-1))/2 ) edges = ((nodes-1)*(nodes))/2;
    if (edges < 0) edges = 0;
    if (threads < 1) threads = 1;
    if (threads > nodes) threads = nodes;
    if (complete) edges = ((nodes-1)*(nodes))/2;
    srand(seed);    

    if (filename_complete == "")
    filename_complete = "input/" + to_string(nodes) + "_" + to_string(edges) + "_" + to_string(seed) + "_" + to_string(connected) + to_string(complete) + to_string(regular) + ".txt";

    ifstream testfile;
    testfile.open(filename_complete);
    if (!testfile.is_open()){
        cerr << "File \033[1m" << filename_complete << "\033[0m does not exist. Please generate the graph first using 'make graph'.\n";
        exit(1);
    }

    graph = new int*[nodes];                                // allocate space for graph
    for(int i=0; i<nodes; i++) graph[i] = new int[nodes](); // initialize to 0

    // thread thread_ids_minur[threads];
    // for (int i = 0; i < threads; i++) {
    //     thread_ids_minur[i] = thread([](const int total){
    //         int now = 0; now = node_num.fetch_add(1);
    //         while (now < total){
    //             for (int j = 0; j < total; j++) graph[now][j] = -1;
    //             now = node_num.fetch_add(1);
    //         }  }, nodes); }
    // for (int i = 0; i < threads; i++) {
    //     thread_ids_minur[i].join(); }

    while (!testfile.eof()){
        int u, v, w;
        testfile >> u >> v >> w;
        if (u >= nodes || v >= nodes) {
            cerr << "Edge (" << u << "," << v << ") in file \033[1m" << filename_complete << "\033[0m exceeds number of nodes " << nodes << ". Please generate the graph first using 'make graph'.\n";
            exit(1);
        }
        if (u == v) continue;
        if (graph[u][v] != 0) continue; // no multi edges
        if (w <= 0) w = 1;
        graph[u][v] = w;
        graph[v][u] = w;
    }
    testfile.close();
}

void MST(int * MST1, int id){
    // lock for condition  barrier
    unique_lock<mutex> lock(mstx);
    //get smallest edge of each node
    uint now = 0;
    now = node_num.fetch_add(1);
    while (now < nodes){
        bool zeroing = 0;
        int first_zero = 0;
        int min1 = INT32_MAX;
        int min2 = INT32_MAX;
        int minloc = -1;
        int minloc2 = -1;
        //on seeing a chain of 0s, make first zero as minus first non zero index.
        for (int i = 0; i < nodes; i++) {
            if (i == now) graph[now][i] = 0;
            if (graph[now][i] > 0){
                if (graph[now][i] < min1) {
                    min2 = min1;
                    minloc2 = minloc;
                    min1 = graph[now][i];
                    minloc = i;
                }
                else if (graph[now][i] < min2 && i != minloc) {
                    min2 = graph[now][i];
                    minloc2 = i;
                }
            }
            if (graph[now][i] == 0) {
                if (!zeroing) {
                    zeroing = 1;
                    first_zero = i;
                }}
            else {
                zeroing = 0;
                graph[now][first_zero] = -i;
            }}
        // printf("Node %d Minimum Edge Weight: %d\n", now, min1);
        meta[now] = -1;      
        mins[now] = minloc;
        MST1[now] = minloc;
        graph[now][now] = minloc2;
        now = node_num.fetch_add(1);
    }
    
    now = done.fetch_add(1);
    if(now == threads ){
        node_num.store(0);
        done.store(0);
        condition_1 = 1;
        condition_2 = 0;
        cv.notify_all();
    }
    cv.wait(lock, []{return condition_1;}); 
    now = node_num.fetch_add(1);
        
        
    while (now < nodes){            
        if (mins[now] != -1)
            if (mins[mins[now]] == now) {
                    if (mins[now] < now) {
                        groupn[conflict.fetch_add(1)] = now;
                    }}

        now = node_num.fetch_add(1);
        now = now - nodes;
    }

    if (id == 0) print_meta();
    
    usleep(10000);

    // pick a conflict as group name and follow its chain to update all nodes in the group

    if ( id > conflict.load()) {
        return;
    }

    int c = conflict.fetch_add(-1);
    while (conflict.load() > 0) {
        int now = groupn[c];
    }


    
    // if (id > groups.load()) {
    //     return;
    // }

    // now = done.fetch_add(1);
    // if(now == threads - 1) {
    //     node_num.store(0);
    //     done.store(0);
    //     condition = 0;
    //     cv.notify_all();
    // }
    // cv.wait(lock, []{return !condition;});
    // while (groups.load() > 1){
    //     now = now % nodes;
    //     //find group of current node
    //     int grp = meta[now];
    // }

    // now = now % nodes;
    // while (groups.load() > 1){
        
    //     now = node_num.fetch_add(1);
    //     now = now % nodes;
    // }

}

int main(int argc, char** argv){
    if (argc > 1)
        filename_complete = argv[1];
    get_ready();    // read input, alllocate space and initialize graph
                    // For fairness no preprocessing is done pure graph in matrix
    //start timer
    
    mins = new int[nodes];         // allocate space for meta data
    meta = new int[nodes];
    groupn = new int[nodes/2];
    // atomic_bool * inMST = new atomic_bool[nodes]();
    int * MST1 = new int[nodes]();
    node_num.store(0);               // reset node counter for threads
    groups.store(nodes);

    thread thread_ids[threads];


    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();


    for (int i = 0; i < threads; i++) {
        thread_ids[i] = thread(MST,  MST1 , i);
    }

    for (int i = 0; i < threads; i++) {
        thread_ids[i].join();
    }

    print_graph();
    print_meta();


    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();


    chrono::duration<double, std::milli> duration = end - start;
    cout << "Time: \033[1m" << (int)duration.count() / 1000 << "." << (int)duration.count() % 1000 << "\033[0m seconds" << endl;
    return 0;
}