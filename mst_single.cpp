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
        int w = weights[u];
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
    //get smallest edge of each node
    int now = 0;
                                        // First phase: find minimum edge for each node
                                        // and set up meta data
    while (now < nodes){                                         
        int min = INT32_MAX;
        int minloc = -1;
        int index = 0;
        for (int i = 0; i < sizess[now]; i++) {// skip used/zero edges
            if (graph[now][i] >= 0) {
                if (graph[now][i] < min) {
                    min = graph[now][i];
                    minloc = index;
                }
                index++;
            }
            else{
                index = -graph[now][i];
            }
        }
        minv[now] = min;
        minl[now] = minloc;
        my_grp[now] = now;
        if (minloc != -1) {
            meta[minloc] = 1;
            order1[now] = minloc;
            order2[minloc] = now;}
        else meta[now] = 1;
        now++;
    }

    now = 0;


    // Second phase: find conflicts 
    //              get leaf nodes of conflict chains
        //              add non-conflict edges to MST

    conflict = 0;

    while (now < nodes){            
        if (minl[now] == -1){
            now = now++;
            my_grp[now] = now;
            continue;
        }
        else if (minl[minl[now]] == now) {
            // cout << "Conflict between nodes " << now << " and " << minl[now] << "\n";
            if (minl[now] < now) {
                    meta[now] = 0;                  // group head
                    meta[minl[now]] = 2;            // probable leaf renamed
                    // my_grp[now] = now;
                    my_grp[minl[now]] = now;
                    order1[minl[now]] = now;
                    order2[minl[now]] = now;
                    order1[now] = now;
                    groupn[conflict] = now;
                    conflict++;
                    MST1[edge] = now;
                    MST2[edge] = minl[now];
                    weights[edge] = minv[now];
                    edge++;
                    total_weight += minv[now];
                }
            }
        else {
            MST1[edge] = now;
            MST2[edge] = minl[now];
            weights[edge] = minv[now];
            edge++;
            total_weight += minv[now];
            my_grp[now] = my_grp[minl[now]];
        }

        if (meta[now] == 0 && minl[minl[now]] != now){
            leaves[leaf_count] = now;
            leaf_count++;
        }
        now++;
    }

    now = 0;

    if (conflict<= 1) return;    
    // return;                           // exit if one group remains
    // Third phase: get conflict chains from leaves

    while (now < leaf_count) {           // get info
        int leaf = leaves[now];
        int curr = leaf;
        order2[leaf] = leaf;
        // curr = minl[curr];
        while (order2[minl[curr]] == curr && order1[curr] != curr){           // traverse chain
            curr = minl[curr];
            order2[curr] = leaf;
        }
        // cout << "Processing chain for leaf " << leaf << "\n";
        if (order1[curr] == curr){
            order2[curr] = leaf;
        }
        
        my_grp[leaf] = my_grp[curr];
        now++;
    }

    now = 0;

    // Fourth phase: update min edges according to groups
    while (now < nodes){            
        if (minl[now] == -1){
            now++;
            continue;
        }
        int grp = my_grp[now];
        while (my_grp[grp] != grp){
            grp = my_grp[grp];
        }
        my_grp[now] = grp;

        int min = INT32_MAX;
        int minloc = -1;
        int i = 0;
        for (int j = 0; j < sizess[now]; j++) {// skip used/zero edges
            if (graph[now][j] >= 0){
                if (graph[now][j] == 0){i++; continue;} // on seeing 0 used
                if (i == now){i++; continue;}
                int my_grp_i = my_grp[i];
                while (my_grp[my_grp_i] != my_grp_i){
                    my_grp_i = my_grp[my_grp_i];
                }
                my_grp[i] = my_grp_i;

                if (my_grp[i] == grp) {
                    graph[now][j] = 0;
                    i++;
                    continue;
                }

                if (graph[now][j] < min){
                    min = graph[now][j];
                    minloc = i;
                }
                i++;
            }
            else{
                i = -graph[now][j];
            }
        }

        if (order1[now] == now){                         //edge case handle
            if(minl[now] != -1 && order2[minl[now]] == now){
            
                if (order2[now] == minl[now]){

                    order2[minl[now]] = minl[now];   
                    order1[minl[now]] = now;
                    my_grp[minl[now]] = now;
                    leaves[leaf_count] = minl[now];
                    leaf_count++;
                }else {
                    order1[minl[now]] = order1[order2[now]];
                    order1[order2[now]] = minl[now];
                    order2[minl[now]] = order2[now];
                }
            }
        }
    
        minv[now] = min;
        minl[now] = minloc;
        now++;
    }


    // return;

    int conflicts_total = 0;
    int conflicts_now = 0;
    int leaves_total = leaf_count;

    // final phase: resolve conflicts between groups
    while (conflict > 1){
        now = 0;

        while (now < leaf_count) {           // get info
            int leaf = leaves[now];
            int curr = leaf;
            int minloc = leaf;
            int minval = minv[leaf];

                if (branch_minv[now] > 0) { now++; continue;}
                // curr = order1[curr];
                while (order2[curr] == leaf){           // get min for that branch            

                    if (minv[curr] < minval){
                        minval = minv[curr];
                        minloc = curr;
                    }
                    if (order1[curr] == curr){break;}
                    curr = order1[curr];
                }
                if (minl[minloc] == -1){
                    minval = INT32_MAX;
                    minloc = -1;
                }
                branch_minl[now] = minloc;
                branch_minv[now] = minval;
                // cout << "Thread " << my_grp[leaf] << " branch starting at leaf " 
                // << leaf << " has minloc " << minloc << " with value " << minval << " going to group " << my_grp[minl[minloc]] << "\n";
                now++;
            }   
            
            
            // get min for the groups
            now = 0;
            conflicts_total += conflicts_now;
            conflicts_now = conflict;
            // cout << conflicts_total << " " << conflicts_now << "\n";
            // return;
            while (now < conflicts_now) {  
                // cout << "Now processing group head " << now + conflicts_total << "\n";
                int group_head = groupn[now + conflicts_total];
                int min = INT32_MAX;
                int minloc = -1;
                // int frmo = 0;
                
                // cout << groupn[now] << " total conflicts now " << conflicts_now << "\n";
                
                //search for min in branch mins
                for (int i = 0; i < leaves_total; i++){
                    int leaf = leaves[i];
                    // cout << "Thread " << group_head << " processing group head " << group_head << "\n";
                    if (my_grp[leaf] != group_head) continue;
                    // cout << "Thread " << group_head << " processing group head " << my_grp[group_head] << "\n";
                    if (branch_minv[i] < min){
                        min = branch_minv[i];
                        minloc = branch_minl[i];
                        // frmo = i;
                    }
                    
                }
                // store in meta 
                meta[group_head] = minloc;
                now = now + 1;
            }
            // return;

            now = 0;
            conflict = 0;
            //print groupn
            //                 for (int i = 0; i < nodes; i++){
            //     cout << groupn[i] << " ";
            // }

            while (now < conflicts_now){            
                int group_head = groupn[now + conflicts_total];
                int minloc = meta[group_head];
                // cout << "Thread " << group_head << " processing group head " << meta[group_head] << "\n";
                if (minloc == -1){
                    now = now + 1;
                    continue;
                }
                int minloc_loc = minl[minloc];              // actual node to which minloc connects in other group
                if (minloc_loc == -1){
                    now = now + 1;
                    continue;
                }
                int minloc_grp = my_grp[minloc_loc];        // group of actual node 
                
                if (group_head == minloc_grp){
                    now = now + 1;
                    continue;
                }
                
                    if (my_grp[minl[meta[minloc_grp]]] == group_head){ // conflict
                        if (group_head > minloc_grp){   // resolve by group head id

                            my_grp[minloc_grp] = group_head;
        
                            MST1[edge] = minloc;
                            MST2[edge] = minloc_loc;
                            weights[edge] = minv[minloc];
                            // weights[edge] = id;
                            edge++;
                            total_weight += minv[minloc];

                            groupn[conflict + conflicts_total + conflicts_now] = group_head;
                            conflict++;
                            // cout << "remain " << conflict << endl;
                        }
                    }
                    else {  // no conflict just add edge
                        MST1[edge] = minloc;
                        MST2[edge] = minloc_loc;
                        weights[edge] = minv[minloc];
                        // weights[edge] = id;
                        edge++;
                        total_weight += minv[minloc];
                        my_grp[group_head] = minloc_grp;
                        // cout << "connecting groups " << group_head << " and " << minloc_grp << "with meight " << minv[minloc] << "\n";
                    }
                now = now + 1;
            }


            if (conflict <= 1) {return; }              // exit if one group remains
            now = 0;

            // return;
            while (now < leaves_total) {         
                // cout << "Thread " << id << " preparing for next iteration.\n";
                int leaf = leaves[now];
                
                // update mins
                int my_grp_leaf = my_grp[leaf];
                int minloc = branch_minl[now];
                if (minloc == -1){
                    now = now + 1;
                    continue;
                }

                while (my_grp[my_grp_leaf] != my_grp_leaf ){ //cout << "my_grp_leaf " << my_grp_leaf << " " << my_grp[my_grp_leaf] << "\n";
                    // usleep(1000);
                    my_grp_leaf = my_grp[my_grp_leaf];}
                my_grp[leaf] = my_grp_leaf;
                
                int minloc_grp = my_grp[minloc];
                while (my_grp[minloc_grp] != minloc_grp){
                    minloc_grp = my_grp[minloc_grp];}
                my_grp[minloc] = minloc_grp;
                
                if (minloc_grp == my_grp_leaf){        // meaning that minloc is in same group as leaf
                                                                        // OR value was used
                    //travel branch and update their  mins
                    branch_minv[now] = 0;
                    int curr = leaf;
                    while (order2[curr] == leaf){
                        if (minl[curr] == -1){
                            if (order1[curr] == curr) break;
                            curr = order1[curr];
                            continue;
                        }
                        int curr_minloc = minl[curr];
                        // cout << "Thread " << id << " updating mins for node " << curr << "\n";
                        if (curr_minloc == -1) {
                            if (order1[curr] == curr) break;
                            curr = order1[curr];
                            continue;
                        }
                        int curr_minloc_grp = my_grp[curr_minloc];
                        
                        while (my_grp[curr_minloc_grp] != curr_minloc_grp){
                            curr_minloc_grp = my_grp[curr_minloc_grp];
                        }
                        my_grp[curr_minloc] = curr_minloc_grp;

                        // curr = order1[curr];
                        // continue;
                        if (my_grp_leaf == curr_minloc_grp ){
                            // graph[curr][minl[curr]] = 0;
                            int min = INT32_MAX;
                            int minloc_new = -1;
                            int index = 0;
                            // cout << "Thread " << id << " updating mins for node " << curr << "\n";
                            for (int i = 0; i < sizess[curr]; i++) {// skip used/zero edges
                                if (graph[curr][i] >= 0){
                                    if (graph[curr][i] == 0){index++; continue;} // on seeing 0 used
                                    if (index == curr){index++; continue;}
                                    if (index >= nodes){break;}
                                    int my_grp_i = my_grp[index];
                                    while (my_grp[my_grp_i] != my_grp_i){
                                        my_grp_i = my_grp[my_grp_i];
                                    }
                                    my_grp[index] = my_grp_i;
                                    
                                    if (my_grp[index] == my_grp_leaf) {
                                        graph[curr][i] = 0;
                                        index++;
                                        continue;
                                    }

                                    if (graph[curr][i] < min){
                                        min = graph[curr][i];
                                        minloc_new = index;
                                    }
                                    index++;
                                }
                                else{
                                    index = -graph[curr][i];
                                    if (index >= nodes){break;}
                                }
                                
                            }
                            // cout << "Thread " << id << " updated mins for node " << curr << " to minloc " << minloc_new << " with value " << min << "\n";
                            if (minloc_new != -1){
                                minl[curr] = minloc_new;
                                minv[curr] = min;
                                // cout << "Updated min for node " << curr << " to node " << minloc_new << " with value " << min << "\n";
                            }
                            else {
                                minl[curr] = -1;
                                minv[curr] = INT32_MAX;
                            }
                        }
                        if (order1[curr] == curr) break;
                        curr = order1[curr];
                    }
                    
                }
                
                now = now + 1;  
                     
            } 
            if (conflict <= 1) return;
            // now = now + 1;
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