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
#include <sstream>
#include <cstring>
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
mutex mstx;
bool condition_1 = 0;
bool condition_2 = 0;
condition_variable cv;

long nodes = 64;
long long edges = 256;
atomic_int threads(4);
atomic_uint node_num(0);
atomic_uint total_weight(0);
atomic_uint edge(0);
atomic_uint done(0);
atomic_uint order(0);
atomic_uint conflict(0);
atomic_uint leaf_count(0);
string filename_complete;

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
        int w = weights[i];
        outfile << u << " " << v << " "  << w << "\n";
    }
    cout << "MST written to file \033[1m" << filename << "\033[0m with total weight " << total_weight.load() << ".\n";
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
        sizess[i] = sizez + edge_count;
        graph[i] = new int32_t[sizess[i]+1];             // int 32 for smaller size
    }


    //read input file to graph matrix
    int i = 0, j = 0, weight = 0;
    while (getline(testfile, line)) {
        istringstream iss(line);
        // parse same way
        while (iss >> j >> paren >> weight >> paren) {
            j = j; // zero based indexing
            if (weight < 0) weight = 1;
            if (last_index[i] + 1 == j){
                graph[i][++last_entry[i]] = weight; 
            }
            else {
                graph[i][++last_entry[i]] = -(j); // mark zeros as negative count
                graph[i][++last_entry[i]] = weight;
            }
            if (last_index[j] + 1 == i){
                graph[j][++last_entry[j]] = weight; 
            }
            else {
                graph[j][++last_entry[j]] = -(i); // mark zeros as negative count
                graph[j][++last_entry[j]] = weight;
            }
            last_index[i] = j;
            last_index[j] = i;
        }      
        i++;
    }

    // // print_graph();
    
    // char * temp;
    // for (int i=0; i < nodes ; i++){
    //     getline(testfile, line);
    //     temp = &line[0];
    //     int total = 1, end = 0, curr = 0;
    //     if (line.length() == 0) continue;
    //     end = line.length();
    //     // cout << "Reading node " << i << "with edge data of length " << end << "\n";
    //     while (total < end ){
    //         while (temp[curr] != '(') curr++;
    //         // temp[curr] = '\0';
    //         j = stoi(temp);
    //         temp = &temp [++curr];
    //         total += curr;
    //         curr = 0;
    //         while (temp[curr] != ')') curr++;
    //         // temp[curr] = '\0';
    //         // cout << line << "\n";
    //         weight = stoi(temp);
    //         temp = &temp [curr+2];
    //         total += curr + 2;
    //         curr = 0;
    //         if (weight < 0) weight = 1;
    //         if (last_index[i] + 1 == j){
    //             graph[i][++last_entry[i]] = weight; 
    //         }
    //         else {
    //             graph[i][++last_entry[i]] = -(j); // mark zeros as negative count
    //             graph[i][++last_entry[i]] = weight;
    //         }
    //         if (last_index[j] + 1 == i){
    //             graph[j][++last_entry[j]] = weight; 
    //         }
    //         else {
    //             graph[j][++last_entry[j]] = -(i); // mark zeros as negative count
    //             graph[j][++last_entry[j]] = weight;
    //         }
    //         last_index[i] = j;
    //         last_index[j] = i;
    //     }
    //     // return 1;
    // }
    
    // print_graph();
    testfile.close();
    delete[] last_entry;
    return 1;
}

void MST(int * MST1, int * MST2 , int id){
    // unique_lock<mutex> lock(mstx);
    int now = 0;
    now = node_num.fetch_add(1);
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
        now = node_num.fetch_add(1);
    }
    
    // barrier
    now = done.fetch_add(1);
    // cout << now << "\n";
    if(now == threads-1){
        node_num.store(0);
        done.store(0);
        condition_1 = 1;
        condition_2 = 0;
        // cv.notify_all();
    }
    // cv.wait(lock, []{return condition_1;}); 
    while (!condition_1) usleep(1);

    // Second phase: find conflicts 
    //              get leaf nodes of conflict chains
        //              add non-conflict edges to MST
    now = node_num.fetch_add(1);
    while (now < nodes){            
        if (minl[now] == -1){
            now = node_num.fetch_add(1);
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
                    groupn[conflict.fetch_add(1)] = now;
                    int this_edge = edge.fetch_add(1);
                    MST1[this_edge] = now;
                    MST2[this_edge] = minl[now];
                    weights[this_edge] = minv[now];
                    // weights[this_edge] = id;
                    total_weight.fetch_add(minv[now]);
                }
            }
        else {
            int this_edge = edge.fetch_add(1);
            MST1[this_edge] = now;
            MST2[this_edge] = minl[now];
            weights[this_edge] = minv[now];
            // weights[this_edge] = id;
            total_weight.fetch_add(minv[now]);
            my_grp[now] = my_grp[minl[now]];
        }

        if (meta[now] == 0 && minl[minl[now]] != now){
            int leaf_index = leaf_count.fetch_add(1);
            leaves[leaf_index] = now;
        }
        now = node_num.fetch_add(1);
    }
    // barrier

    now = done.fetch_add(1);
    if(now == threads-1 ){
        node_num.store(0);
        done.store(0);
        condition_1 = 0;
        condition_2 = 1;
        // cv.notify_all();   
    }
    // cv.wait(lock, []{return condition_2;}); 
    while (!condition_2) usleep(1);
    // return;
    if (conflict.load() <= 1) return;        
    // return;                       // exit if one group remains
    // Third phase: get conflict chains from leaves
    
    int leaves_total = leaf_count.load();

    now = node_num.fetch_add(1);
    while (now < leaves_total) {           // get info
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
        now = node_num.fetch_add(1);
    }

    now = done.fetch_add(1);
    if(now >= threads.load() -1 ){
        node_num.store(0);
        done.store(0);
        condition_1 = 1;
        condition_2 = 0;
        // cv.notify_all();        
    }
    // cv.wait(lock, []{return condition_1;}); 
    while (!condition_1) usleep(1);
    now = node_num.fetch_add(1);
    // return;

    // Fourth phase: update min edges according to groups
    while (now < nodes){            
        if (minl[now] == -1){
            now = node_num.fetch_add(1);
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
                    leaves[leaf_count.fetch_add(1)] = minl[now];
                }else {
                    order1[minl[now]] = order1[order2[now]];
                    order1[order2[now]] = minl[now];
                    order2[minl[now]] = order2[now];
                }
            }
        }
        
    
        minv[now] = min;
        minl[now] = minloc;
        now = node_num.fetch_add(1);
    }
    // cout << conflict.load() << " groups remain.\n";
    // return;
    // barrier
    now = done.fetch_add(1);
    if(now >= threads.load() -1 ){
        node_num.store(0);
        done.store(0);
        condition_1 = 0;
        condition_2 = 1;
        cv.notify_all();
        // print_meta();
        // print_graph();
    }
    // cv.wait(lock, []{return condition_2;}); 
    while (!condition_2) usleep(1);
    leaves_total = leaf_count.load();

    // Final phase:  iterate till no conflicts remain
    int conflicts_now = 0;
    int conflicts_total = 0;
    // return;
    // cout << conflict.load() << " groups remain.\n";
    while (conflict.load() > 1){
        now = node_num.fetch_add(1);
        
        while (now < leaves_total) {           // get info
            int leaf = leaves[now];
            int curr = leaf;
            int minloc = leaf;
            int minval = minv[leaf];
            // int minloc_grp = my_grp[minloc];
            // cout << "Thread " << id << 
            //     " processing branch starting at leaf " << leaf << "\n";
                if (branch_minv[now] > 0) { now = node_num.fetch_add(1); continue;}
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
                now = node_num.fetch_add(1);
            }   
            
            // barrier
            now = done.fetch_add(1);
            if(now >= threads.load() -1 ){
                node_num.store(0);
                done.store(0);
                condition_1 = 1;
                condition_2 = 0;
                // cv.notify_all();
                // print_graph();
                // print_meta();
            }
            // cv.wait(lock, []{return condition_1;}); 
            while (!condition_1) usleep(1);
            // return;
            
            // get min for the groups
            now = node_num.fetch_add(1);
            conflicts_total += conflicts_now;
            conflicts_now = conflict.load();
            // return;
            while (now < conflicts_now) {    
                int group_head = groupn[now + conflicts_total];
                int min = INT32_MAX;
                int minloc = -1;
                // int frmo = 0;
                                
                //search for min in branch mins
                for (int i = 0; i < leaves_total; i++){
                    int leaf = leaves[i];
                    if (my_grp[leaf] != group_head) continue;
                    if (branch_minv[i] < min){
                        min = branch_minv[i];
                        minloc = branch_minl[i];
                        // frmo = i;
                    }
                    
                }
                // store in meta 
                meta[group_head] = minloc;
                now = node_num.fetch_add(1);
            }
            
            // barrier
            // return;
            now = done.fetch_add(1);
            if(now >= threads.load() -1 ){
                node_num.store(0);
                done.store(0);
                conflict.store(0);
                condition_1 = 0;
                condition_2 = 1;
                // cv.notify_all();


            }
            // cv.wait(lock, []{return condition_2;});
            while (!condition_2) usleep(1);
            // cout << conflict.load() << " groups remain.\n";
            
            // now gt conflicts again between groups this time
            now = node_num.fetch_add(1);
            while (now < conflicts_now){            
                int group_head = groupn[now + conflicts_total];
                int minloc = meta[group_head];
                // cout << "Thread " << group_head << " processing group head " << meta[group_head] << "\n";
                if (minloc == -1){
                    now = node_num.fetch_add(1);
                    continue;
                }
                int minloc_loc = minl[minloc];              // actual node to which minloc connects in other group
                if (minloc_loc == -1){
                    now = node_num.fetch_add(1);
                    continue;
                }
                int minloc_grp = my_grp[minloc_loc];        // group of actual node 
                
                if (group_head == minloc_grp){
                    now = node_num.fetch_add(1);
                    continue;
                }
                
                    if (my_grp[minl[meta[minloc_grp]]] == group_head){ // conflict
                        if (group_head > minloc_grp){   // resolve by group head id

                            my_grp[minloc_grp] = group_head;
                            int this_edge = edge.fetch_add(1);
                            MST1[this_edge] = minloc;
                            MST2[this_edge] = minloc_loc; 
                            weights[this_edge] = minv[minloc];
                            // weights[this_edge] = id;
                            total_weight.fetch_add(minv[minloc]);
                            // cout << "Conflict resolved, " << old_conflict << " groups remain.\n";
                            groupn[conflict.fetch_add(1) + conflicts_total + conflicts_now] = group_head;
                            // cout << "connecting groups " << group_head << " and " << minloc_grp << "\n";
                        }
                    }
                    else {  // no conflict just add edge

                        int this_edge = edge.fetch_add(1);
                        MST1[this_edge] = minloc;
                        MST2[this_edge] = minloc_loc;
                        weights[this_edge] = minv[minloc];
                        // weights[this_edge] = id;
                        total_weight.fetch_add(minv[minloc]);
                        my_grp[group_head] = minloc_grp;
                        // cout << "connecting groups " << group_head << " and " << minloc_grp << "with meight " << minv[minloc] << "\n";
                    }
                now = node_num.fetch_add(1);
            }
                // return;//////////////////
            now = done.fetch_add(1);
            if(now >= threads.load() -1 ){
                node_num.store(0);
                done.store(0);
                condition_1 = 1;
                condition_2 = 0;
                // cv.notify_all();
            }
            // cv.wait(lock, [] { return condition_1; });/
            while (!condition_1) usleep(1);

            if (conflict <= 1) {return; }              // exit if one group remains
            
            // prepare for next iteration
            // get new groups for chains 
            // get new mins if min was in same group
            // leaves are same as before

            now = node_num.fetch_add(1);
            // return;
            while (now < leaves_total) {         
                int leaf = leaves[now];
                
                // update mins
                int my_grp_leaf = my_grp[leaf];
                int minloc = branch_minl[now];
                if (minloc == -1){
                    now = node_num.fetch_add(1);
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
                
                now = node_num.fetch_add(1);   
                     
            } 
            if (conflict.load() <= 1) return;
            now = done.fetch_add(1);
            if(now >= threads.load() -1 ){
                node_num.store(0);
                done.store(0);
                condition_1 = 0;
                condition_2 = 1;
                // cv.notify_all();    
            }
            // cv.wait(lock, []{return condition_2;});
            while (!condition_2) usleep(1);
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
    // return 1;

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
    node_num.store(0);               // reset node counter for threads

    int thread_temp = threads.load();
    thread thread_ids[thread_temp];


    cout << nodes << " nodes, " << edges << " edges, " << threads.load() << " threads.\n";
    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

    for (int i = 0; i < thread_temp; i++) {
        thread_ids[i] = thread(MST,  MST1, MST2  , i);
    }

    for (int i = 0; i < thread_temp; i++) {
        thread_ids[i].join();
    }

    // print_graph();
    // print_meta();
    cout << edge.load() << " edges in MST.\n";
    
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

    chrono::duration<double> duration = end - start;
    cout << "Time: \033[1m" << (double)duration.count() << "\033[0m seconds" << endl;
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