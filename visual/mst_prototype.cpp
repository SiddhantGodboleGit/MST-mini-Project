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
#define metyr 1

int ** graph;
int * minl;
int * minv;
int * meta;
int * my_grp;
int * groupn;
int * leaves;
int * branch_minl;
int * branch_minv;
int * order1;
int * order2;
mutex mstx;
bool condition_1 = 0;
bool condition_2 = 0;
condition_variable cv;

long nodes = 64, edges = 256;
atomic_uint threads = 4;
atomic_uint node_num(0);
atomic_uint total_weight(0);
atomic_uint edge(0);
atomic_uint done(0);
atomic_uint order(0);
atomic_uint conflict(0);
atomic_uint leaf_count(0);
string filename_complete;

// Function to write MST to file
/* This function writes the Minimum Spanning Tree (MST) edges to a file. */
void mst_to_file(int * MST1, int * MST2){
    string filename = filename_complete;
    filename = filename.substr(0, filename.find_last_of('.')) + "_mst.txt";
    //replace input with output
    filename = "output/" + filename.substr(filename.find_last_of('/') + 1);
    ofstream outfile;
    outfile.open(filename , std::ios::out | std::ios::trunc);
    int total = edge.load();
    for (int i = 0; i < total; i++){
        int u = MST1[i];
        int v = MST2[i];
        outfile << u << " " << v << " "  << "\n";
    }
    cout << "MST written to file \033[1m" << filename << "\033[0m with total weight " << total_weight.load() << ".\n";
    outfile.close();
    return;
}

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
    if (edges > ((nodes)*(nodes-1.0))/2 ) edges = ((nodes-1)*(nodes))/2;
    if (edges < 0) edges = 0;
    if (thread_temp < 1) thread_temp = 1;
    if (thread_temp > nodes) thread_temp = nodes;
    if (complete) edges = ((nodes-1)*(nodes))/2;
    threads.store(thread_temp);
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
            return 0;
        }
        if (u == v) continue;
        if (graph[u][v] != 0) continue; // no multi edges
        if (w <= 0) w = 1;
        graph[u][v] = w;
        graph[v][u] = w;
    }
    testfile.close();
    return 1;
}

void MST(int * MST1, int * MST2, int id){
    // lock for condition  barrier
    unique_lock<mutex> lock(mstx);
    //get smallest edge of each node
    uint now = 0;
    now = node_num.fetch_add(1);
                                        // First phase: find minimum edge for each node
                                        // and set up meta data
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
                if (zeroing == 1){
                zeroing = 0;
                graph[now][first_zero] = -i;
                }
            }}
        // printf("Node %d Minimum Edge Weight: %d\n", now, min1);
        minv[now] = min1;
        minl[now] = minloc;
        my_grp[now] = now;
        // meta[minloc] = 1;
        graph[now][minloc] = -(minloc + 1 ); //-(min1 + nodes);  // mark as used
        if (minloc2 != -1)
            graph[now][now] = minloc2;
        now = node_num.fetch_add(1);
    }
    
    // barrier
    now = done.fetch_add(1);
    if(now == threads -1){
        node_num.store(0);
        done.store(0);
        condition_1 = 1;
        condition_2 = 0;
        cv.notify_all();
    }
    cv.wait(lock, []{return condition_1;}); 
    now = node_num.fetch_add(1);

    while (now < nodes){            
        // set the meta
        if (minl[minl[now]] == now) {
            if (minl[now] < now) meta[now] = 5;          // mark as conflict chain head
            else {
                meta[now] = 0;
            }
        }
        else {
            meta[minl[now]] = 1;
        }
        now = node_num.fetch_add(1);
    }
        
    // Second phase: find conflicts 
    //              get leaf nodes of conflict chains
    now = now - nodes;
    while (now < nodes){            
        if (minl[now] == -1){
            now = node_num.fetch_add(1) - nodes;
            meta[now] = 1;
            my_grp[now] = now;
            continue;
        }
        else if (minl[minl[now]] == now) {
            if (minl[now] < now) {
                        // meta[now] = 5;
                        my_grp[now] = now;
                        my_grp[minl[now]] = now;
                        groupn[conflict.fetch_add(1)] = now;
                        int this_edge = edge.fetch_add(1);
                        MST1[this_edge] = now;
                        MST2[this_edge] = minl[now];
                        total_weight.fetch_add(minv[now]);
                        graph[now][minl[now]] = 0;
                        graph[minl[now]][now] = 0;
                        if (metyr == 2){
                            cout << "Conflict between nodes " << now << " and " << minl[now] << "\n";
                        }
                    }
                }
        else {
            int this_edge = edge.fetch_add(1);
            MST1[this_edge] = now;
            MST2[this_edge] = minl[now];
            total_weight.fetch_add(minv[now]);
            graph[now][minl[now]] = 0;
            graph[minl[now]][now] = 0;
            if (metyr ==2 ){
                cout << "Adding edge between nodes " << now << " and " << minl[now] << "\n";
            }
            my_grp[now] = my_grp[minl[now]];
        }

        now = node_num.fetch_add(1) - nodes;
    }

    now = node_num.fetch_add(1) - (nodes<<1);
    while (now < nodes){            
        if (meta[now] == 0){               // leaf node
            int leaf_index = leaf_count.fetch_add(1);
            leaves[leaf_index] = now;
        }
        now = node_num.fetch_add(1) - (nodes<<1);
    }
    
    // barrier
    now = done.fetch_add(1);
    if(now == threads -1 ){
        node_num.store(0);
        done.store(0);
        condition_1 = 0;
        condition_2 = 1;
        cv.notify_all();   
        // print_meta();
        // for (int i = 0; i < nodes; i++){
        //     cout << "group of node " << i << " is " << my_grp[i] << "\n";}
    }
    cv.wait(lock, []{return condition_2;}); 
    
    // Third phase: get groups of conflict chains from leaves
    
    int leaves_total = leaf_count.load();
    // if (id >= leaves_total){ 
    //     threads.fetch_sub(1);
    //     return;}
        
    now = node_num.fetch_add(1);
    while (now < leaves_total) {           // get info
        int leaf = leaves[now];
        int curr = leaf;

        curr = my_grp[curr];
        while (meta[curr] == 1){
            meta[curr] = -(leaf);          // mark as visited 
            curr = my_grp[curr];        // find group head
        }

        if (meta[curr] < 0 && leaf != -meta[curr]){            // already assigned by another leaf
            my_grp[leaf] = -meta[curr];
        }
        else                            // new group
            my_grp[leaf] = curr;
        now = node_num.fetch_add(1);
    }

    now = done.fetch_add(1);
    if(now >= threads -1 ){
        node_num.store(0);
        done.store(0);
        condition_1 = 1;
        condition_2 = 0;
        cv.notify_all();
                    //             for (int i = 0; i < nodes; i++){
                    // cout << "group of node " << i << " is " << my_grp[i] << "\n";}
    }
    cv.wait(lock, []{return condition_1;}); 
    now = node_num.fetch_add(1);

    // Fourth phase: assign groups to conflict chains
    now = now;
    while (now < leaves_total) {           // assign groups)    
        int leaf = leaves[now];
        int curr = leaf;
        int grp = my_grp[leaf];

        while (my_grp[grp] != grp){   // path compression
            grp = my_grp[grp];
        }

        my_grp[leaf] = grp;
        // cout << "Thread " << id << " assigning group " << grp << " to chain starting at leaf " << leaf << "\n";

        curr = minl[leaf];
        order2[leaf] = leaf;
        while (my_grp[curr] != grp){        // assign group to chain
            // cout << "Thread " << id << " assigning group " << grp << " to node " << curr << " in chain starting at leaf " << leaf << "\n";
            order2[curr] = leaf;
            my_grp[curr] = grp;
            curr = minl[curr];
        }

        now = node_num.fetch_add(1);
    }

    // barrier
    now = done.fetch_add(1);
    if(now >= threads.load() -1 ){
        node_num.store(0);
        done.store(0);
        condition_1 = 0;
        condition_2 = 1;
        cv.notify_all();
                    //             for (int i = 0; i < nodes; i++){
                    // cout << "group of node " << i << " is " << my_grp[i] << "\n";}
        
    }
    cv.wait(lock, []{return condition_2;}); 
    now = node_num.fetch_add(1);
    // return;
    // FiFth phase: update min edges according to groups
    while (now < nodes){            
        if (minl[now] == -1){
            now = node_num.fetch_add(1);
            continue;
        }
        int grp = my_grp[now];
        int minloc = graph[now][now];
        int minval = graph[now][minloc];


        // check if minloc is in same group
        if (my_grp[minloc] == grp){
            // find new min edge outside group
            int min1 = INT32_MAX;
            int min2 = INT32_MAX;
            int minloc1 = -1;
            int minloc2 = -1;
            for (int i = 0; i < nodes; i++) {// skip used/zero edges
                if (graph[now][i] < 0){ 
                    int j = -graph[now][i];
                    while (graph[now][j] < 0)
                         j = -graph[now][j];
                    graph[now][i] = -j;
                    i = j;
                    }
                if (graph[now][i] == 0) continue; // on seeing 0 no more edges     
                if (i == now) continue;
                if (my_grp[i] == grp) {
                    // graph[now][i] = -graph[now][i];
                    continue;
                }

                if (graph[now][i] < min1) {
                    min2 = min1;
                    minloc2 = minloc1;
                    min1 = graph[now][i];
                    minloc1 = i;
                }
                else if (graph[now][i] < min2 && i != minloc1) {
                    min2 = graph[now][i];
                    minloc2 = i;
                }}

                minloc = minloc1;
                minval = min1;
                if (minloc2 != -1) 
                    graph[now][now] = minloc2;
            }
        
        minv[now] = minval;
        if(my_grp[now] != now) order1[now] = minl[now];
        else order1[now] = now;
        minl[now] = minloc;

        now = node_num.fetch_add(1);
    }

    // compress path of all
    now = node_num.fetch_add(1) - (nodes);
    while (now < nodes){            
        int grp = my_grp[now];
        while (my_grp[grp] != grp){
            grp = my_grp[grp];
        }
        my_grp[now] = grp;
        now = node_num.fetch_add(1) - (nodes);
    }
    // barrier
    now = done.fetch_add(1);
    if(now >= threads.load() -1 ){
        node_num.store(0);
        done.store(0);
        condition_1 = 1;
        condition_2 = 0;
        cv.notify_all();
        // print_meta();
        // print_graph();
    }
    cv.wait(lock, []{return condition_1;}); 
    
    // exit if more than group number
    // if (id >= conflict){ 
    //     threads.fetch_sub(1);
    //     return;}
        
        // final phase: loop till one group remains

    int conflicts_now = 0;
    // cout << conflict.load() << " groups remain.\n";
    while (conflict.load() > 1){
        now = node_num.fetch_add(1);
        // if (id >= conflict){ 
        // threads.fetch_sub(1);
        // return;}
            while (now < leaves_total) {           // get info
                int leaf = leaves[now];
                int curr = leaf;
                int minloc = leaf;
                int minval = minv[leaf];
                int minloc_grp = my_grp[minloc];
                if (metyr == 2 ) cout << "Thread " << id << " processing branch starting at leaf " << leaf << "\n";
                if (branch_minv[now] > 0) { now = node_num.fetch_add(1); continue;}
                while (order2[curr] == leaf){           // get min for that branch                
                    // if (metyr == 2 ) cout << "Thread " << id << " traversing branch starting at leaf " << leaf << " visiting node " << curr << "\n";
                    if (minv[curr] < minval){
                        minval = minv[curr];
                        minloc = curr;
                    }
                    if (order1[curr] == curr) break;
                    curr = order1[curr];
                }
                if (minl[minloc] == -1){
                    minval = INT32_MAX;
                    minloc = -1;
                }
                branch_minl[now] = minloc;
                branch_minv[now] = minval;
                // cout << "Thread " << id << " branch starting at leaf " << leaf << " has minloc " << minloc << " with value " << minval << "\n";
                now = node_num.fetch_add(1);
            }   

            // barrier
            now = done.fetch_add(1);
            if(now >= threads.load() -1 ){
                node_num.store(0);
                done.store(0);
                condition_1 = 0;
                condition_2 = 1;
                cv.notify_all();
                // print_graph();
            }
            cv.wait(lock, []{return condition_2;}); 
            // get min for the groups
            now = node_num.fetch_add(1);
            conflicts_now = conflict.load();
            // return;
            while (now < conflicts_now) {    
                // cout << "Thread " << id << " processing group head " << now << "\n";  
                int group_head = groupn[now];
                int min = INT32_MAX;
                int minloc = -1;
                // int minloc_grp = my_grp[minloc];
                // if (min == 0){
                    //get min for group head
                    // min = INT32_MAX;
                    // minloc = -1;
                //     for (int i = 0; i < nodes; i++) {// skip used/zero edges
                //         if (graph[group_head][i] < 0){ 
                //             int j = -graph[group_head][i];
                //             // cout << "Thread " << id << " checking edge from group head " << group_head << " to node " << i << "\n";
                //             while (graph[group_head][j] < 0)
                //             j = -graph[group_head][j];
                //             graph[group_head][i] = -j;
                //             if (i < j)i = j;
                //             // cout << "Thread " << id << " edge weight is " << graph[group_head][i] << "\n";
                //             // cout << "Thread " << id << " group of node " << i << " is " << my_grp[i] << "\n";
                //         }
                //         if (graph[group_head][i] == 0) continue;    
                //         if (i == group_head) continue;
                //         int my_grp_i = my_grp[i];
                //         while (my_grp[my_grp_i] != my_grp_i){
                //             my_grp_i = my_grp[my_grp_i];
                //         }
                //         my_grp[i] = my_grp_i;
                //         if (my_grp_i == group_head) continue;
                //         if (graph[group_head][i] < min){
                //             min = graph[group_head][i];
                //             minloc = i;
                //         }
                //     // }
                //     minv[group_head] = min;
                //     minl[group_head] = minloc;
                // }
                // int head_min = minl[group_head];
                // //check if minloc is in same group
                // while (my_grp[head_min] != head_min){
                //     head_min = my_grp[head_min];
                // }
                // if (my_grp[head_min] == group_head){
                //     min = INT32_MAX;
                //     minloc = -1;
                // }
                
                //search for min in branch mins
                for (int i = 0; i < leaves_total; i++){
                    int leaf = leaves[i];
                    if (my_grp[leaf] != group_head) continue;
                    if (branch_minv[i] < min){
                        min = branch_minv[i];
                        minloc = branch_minl[i];
                    }
                }
                
                // store in meta 
                meta[group_head] = minloc;
                now = node_num.fetch_add(1);
            }
            
            // cout << "Thread " << id << " completed branch mins.\n";
            // return;
            // barrier
            now = done.fetch_add(1);
            if(now >= threads.load() -1 ){
                node_num.store(0);
                done.store(0);
                conflicts_now = conflict.load();
                conflict.store(0);
                condition_1 = 1;
                condition_2 = 0;
                cv.notify_all();
                // for (int i = 0; i < nodes; i++){
                //     cout << "group of node " << i << " is " << my_grp[i] << "\n";}
                // for (int i = 0; i < conflicts_now; i++){
                //     int group_head = groupn[i];
                //     int minloc = meta[group_head];
                //     cout << "Group head " << group_head << " has minloc " << minloc << " to minloc_loc " << minl[minloc] << " with value " << minv[minloc] << "\n";}
                // for (int i = 0; i < leaves_total; i++){
                //     int leaf = leaves[i];
                //     cout << "Branch starting at leaf " << leaf << " has minloc " << branch_minl[i] << " with value " << branch_minv[i] << "\n";}
                // cout << conflicts_now << " groups remain.\n";
                // cout << leaves_total << " leaves total.\n";
            }
            cv.wait(lock, []{return condition_1;});

            // now gt conflicts again between groups this time
            now = node_num.fetch_add(1);
            while (now < conflicts_now){            
                int group_head = groupn[now];
                int minloc = meta[group_head];
                if (minloc == -1){
                    now = node_num.fetch_add(1);
                    continue;
                }
                cout << "Thread " << id << " group head " << group_head << " has minloc " << minloc << " with value " << minv[minloc] << "\n";
                int minloc_loc = minl[minloc];              // actual node to which minloc connects in other group
                if (minloc_loc == -1){
                    now = node_num.fetch_add(1);
                    continue;
                }
                // cout << "Thread " << id << " processing group head " << group_head << "\n";
                // usleep (10000);
                int minloc_grp = my_grp[minloc_loc];        // group of actual node 
                // cout << "Thread " << id << " compressing group for node " << minloc_grp << "\n";
                // usleep (10000);
                // while (my_grp[minloc_grp] != minloc_grp){ // not a group head
                //     minloc_grp = my_grp[minloc_grp];
                // }
                // cout << "Thread " << id << " checking conflict between groups " << group_head << " and " << minloc_grp << "for edge between nodes " << minloc << " and " << minloc_loc << "\n";

                if (group_head == minloc_grp){
                    // cout << "Thread " << id << " no conflict for group head " << group_head << " for edge between nodes " << minloc << " and " << minloc_loc << "\n";
                    // conflict.fetch_add(1);
                    now = node_num.fetch_add(1);
                    continue;
                }
                //check on conflict
                if (metyr == 2){
                // cout << "Thread " << id << " checking conflict between groups " << group_head << " and " << minloc_grp << "for edge between nodes " << minloc << " and " << minloc_loc << "\n";
                // for (int i = 0; i < nodes; i++){
                //     cout << "group of node " << i << " is " << my_grp[i] << "\n";}
                }
                if (my_grp[minl[meta[minloc_grp]]] == group_head){ // conflict
                    if (group_head > minloc_grp){   // resolve by group head id
                        // cout << "Thread " << id << " resolving conflict between groups " << group_head << " and " << minloc_grp << " for edge between nodes " << minloc << " and " << minloc_loc << "\n";
                        my_grp[minloc_grp] = group_head;
                        int this_edge = edge.fetch_add(1);
                        MST1[this_edge] = minloc;
                        MST2[this_edge] = minloc_loc; 
                        total_weight.fetch_add(minv[minloc]);
                        groupn[conflict.fetch_add(1)] = group_head;
                        graph[minloc][minloc_loc] = 0;
                        graph[minloc_loc][minloc] = 0;
                    }
                }
                else {  // no conflict just add edge
                    int this_edge = edge.fetch_add(1);
                    MST1[this_edge] = minloc;
                    MST2[this_edge] = minloc_loc;
                    total_weight.fetch_add(minv[minloc]);
                    my_grp[group_head] = minloc_grp;
                    graph[minloc][minloc_loc] = 0;
                    graph[minloc_loc][minloc] = 0;
                }
                now = node_num.fetch_add(1);
            }
            now = done.fetch_add(1);
            if(now >= threads.load() -1 ){
                node_num.store(0);
                done.store(0);
                condition_1 = 0;
                condition_2 = 1;
                cv.notify_all();
            }
            cv.wait(lock, []{return condition_2;});
            if (conflict.load() <= 1) return;                               // exit if one group remains
            // prepare for next iteration
            // get new groups for chains 
            // get new mins if min was in same group
            // leaves are same as before
            now = node_num.fetch_add(1);
            while (now < leaves_total) {           // get info
                int leaf = leaves[now];
                int curr = leaf;
                while (my_grp[curr] != curr) {
                    curr = my_grp[curr];
                }
                my_grp[leaf] = curr;
                
                // update mins
                int minloc = branch_minl[now];
                int minval = branch_minv[now];
                int minloc_grp = my_grp[minloc];
                int my_grp_leaf = my_grp[leaf];
                while (my_grp[minloc_grp] != minloc_grp){
                    minloc_grp = my_grp[minloc_grp];
                }
                my_grp[minloc] = minloc_grp;
                while (my_grp[my_grp_leaf] != my_grp_leaf){
                    my_grp_leaf = my_grp[my_grp_leaf];
                }
                my_grp[leaf] = my_grp_leaf;
                if (minloc_grp == my_grp[leaf]){        // meaning that minloc is in same group as leaf
                    //travel branch and update their  mins
                    branch_minv[now] = 0;
                    int curr = leaf;
                    // cout << "Thread " << id << " updating mins for branch starting at leaf " << leaf << "\n";
                    while (order2[curr] == leaf){
                        //check if each node's min's group is different that node's group
                        // cout << curr << " has minloc " << order2[curr] << "\n";
                        int curr_minloc = minl[curr];
                        int curr_minloc_grp = my_grp[curr_minloc];
                        while (my_grp[curr_minloc_grp] != curr_minloc_grp){
                            // cout << "Thread " << id << " compressing group for node " << curr_minloc_grp << " from node " << curr << "\n";
                            curr_minloc_grp = my_grp[curr_minloc_grp];
                        }
                        my_grp[curr_minloc] = curr_minloc_grp;
                        // cout << "Thread " << id << " checking node " << curr << " with minloc " << curr_minloc << " in group " << curr_minloc_grp << " against leaf group " << my_grp_leaf << "\n";
                        if (my_grp_leaf == curr_minloc_grp ){
                            int min = INT32_MAX;
                            int minloc_new = -1;
                            for (int i = 0; i < nodes; i++) {// skip used/zero edges
                                if (graph[curr][i] < 0){ 
                                    int j = -graph[curr][i];
                                    while (graph[curr][j] < 0)
                                    j = -graph[curr][j];
                                    graph[curr][i] = -j;
                                    if (i < j)i = j;
                                    }
                                    if (i == curr) continue;
                                    int my_grp_i = my_grp[i];
                                    while (my_grp[my_grp_i] != my_grp_i){
                                        my_grp_i = my_grp[my_grp_i];
                                    }
                                    my_grp[i] = my_grp_i;
                                    if (my_grp_i == my_grp_leaf) continue;
                                    if (graph[curr][i] < min){
                                        min = graph[curr][i];
                                        minloc_new = i;
                                    }
                                
                                // cout << "Thread setting new min for node " << curr << " to " << minloc_new << " with value " << min << "\n";
                            }
                            if (minloc_new != -1){
                                minl[curr] = minloc_new;
                                minv[curr] = min;
                            }
                            // else {
                            //     minl[curr] = -1;
                            //     minv[curr] = INT32_MAX;
                            // }
                            // cout << "Thread " << id << " updated minloc for node " << curr << " to " << minloc_new << " with value " << min << "\n";
                        }
                        if (order1[curr] == curr) break;
                        curr = order1[curr];
                    }
                }
                now = node_num.fetch_add(1);        
            }
            // cout << "One iteration complete. " << conflict.load() << " groups remain.\n";  
            
            now = done.fetch_add(1);
            if(now >= threads.load() -1 ){
                node_num.store(0);
                done.store(0);
                condition_1 = 1;
                condition_2 = 0;
                cv.notify_all();    
                // for (int i = 0; i < nodes; i++){
                //     cout << "group of node " << i << " is " << minl[i] << "\n";}
            }
            cv.wait(lock, []{return condition_1;});
            // if (id == 0) cout << conflict.load() << " groups remain.\n";
            // usleep(10000);
            // return;
            
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
    branch_minl = new int[nodes];
    order1 = new int[nodes];
    groupn = new int[nodes];
    order2 = new int[nodes];
    branch_minv = new int[nodes]();
    // atomic_bool * inMST = new atomic_bool[nodes]();
    int * MST1 = new int[nodes]();
    int * MST2 = new int[nodes]();
    node_num.store(0);               // reset node counter for threads

    int thread_temp = threads.load();
    thread thread_ids[thread_temp];


    cout << nodes << " nodes, " << edges << " edges, " << threads.load() << " threads.\n";
    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();


    for (int i = 0; i < thread_temp; i++) {
        thread_ids[i] = thread(MST,  MST1, MST2, i);
    }

    for (int i = 0; i < thread_temp; i++) {
        thread_ids[i].join();
    }

    // print_graph();
    // print_meta();
    cout << edge.load() << " edges in MST.\n";
    
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();


    mst_to_file(MST1, MST2);



    chrono::duration<double, std::milli> duration = end - start;
    cout << "Time: \033[1m" << (int)duration.count() / 1000 << "." << (int)duration.count() % 1000 << "\033[0m seconds" << endl;
    return 0;
}