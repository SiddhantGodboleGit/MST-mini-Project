/*{
    "success": "0",
    "seed": "0",
    "nodes": "64",
    "edges": "26",
    "density": "0.05",
    "connected": true,
    "complete": false,
    "regular": false
}*/

// read the input_params.txt then generate a random graph accordingly
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <thread>

using namespace std;

int ** matrix;
int *meta;
int *sizes;
int *last_index;
long success, seed, nodes, edges;
double density;
bool connected, complete, regular;
int * edgesize;
int done = 0;

int max_weight = 1000;

void print_graph(){
    cout << "Graph adjacency matrix:\n";
    for (int i = 0; i < nodes; i++){
        for (int j = 0; j < nodes - i; j++){
            cout << matrix[i][j] << " ";
        }
        cout << "\n";
    }
}

void spanit(){
    //make a random spanning tree
    //choose a node at random
    //choose another node at random and connect it to one of the connected nodes
    vector<int> connected;
    vector<int> unconnected;
    int connected_nodes = 1;
    int unconnected_nodes = nodes;

    for (int i = 0; i < nodes; i++){
        unconnected.push_back(i);
    }

    int now = rand() % nodes;        
    
    connected.push_back(unconnected[now]);
    unconnected_nodes--;
    unconnected[now] = unconnected[unconnected_nodes];
    // for (int i = 0; i < unconnected_nodes; i++){
    //         cout << unconnected[i] << " ";
    //     }
    while (connected_nodes < nodes){
        int white_loc = rand() % unconnected_nodes;
        int white = unconnected[white_loc];
        int red;
        do {
            red = connected[rand() % connected_nodes];
        } while (red == white);

        // cout << "Connecting " << white << " and " << red << endl;
        // for (int i = 0; i < unconnected_nodes; i++){
        //     cout << unconnected[i] << " ";
        // }
        // cout << endl;
        
        if (white < red) {
            int temp = red;
            red = white;
            white = temp;
        }
        matrix[red][white - red] = -1;
        
        matrix[white][0]++;
        matrix[red][0]++;
        done++;
        //         cout << "Adding edge between " << red << " and " << white << "\n";
        //         cout << matrix[red][white - red] << "\n";
        // for (int i = 0; i < nodes; i++){
        //     for (int j = 0; j < nodes - i; j++){
        //         cout << matrix[i][j] << " ";
        //     }
        //     cout << "\n";
        // }
        
        connected.push_back(unconnected[white_loc]);
        unconnected_nodes--;
        unconnected[white_loc] = unconnected[unconnected_nodes];
        connected_nodes++;
    }
    cout << "Spanning tree created with " << connected_nodes << " nodes and " << done << " edges.\n";
    return;
}

void make_regular(){
    // generate fixed number of edges for each node
    int target_degree = edges / nodes;
    for(int i = 0; i < nodes; i++){
        //connect to neighbours with distance curret degree
        for (int j = 1; j <= target_degree / 2; j++){
            int neighbor = (i + j) % nodes;
            if (i < neighbor){
                matrix[i][neighbor - i] = -2;
            }
            else {
                matrix[neighbor][i - neighbor] = -2;
            }
            done++;
        }
    }
}

void gen_edge(){
    //generate edges
    // for (int i = 0 ; i < nodes ; i ++){
    //     matrix[i][0] = 0;
    // }
    // cout << "Generating edges...\n";
    while(done < edges){
        int white = rand() % nodes;
        int red;
        do {
            red = rand() % nodes;
        } while (red == white);

        int temp;
        if (white < red){
            temp = red;
            red = white;
            white = temp;
        }
        // cout << "Adding edge between " << red << " and " << white << "\n";
        // for (int i = 0; i < nodes; i++){
        //     for (int j = 0; j < nodes - i; j++){
        //         cout << matrix[i][j] << " ";
        //     }
        //     cout << "\n";
        // }
        if (matrix[red][white - red] == 0){
            matrix[red][white - red] = -2;
            matrix[white][0]++;
            matrix[red][0]++;
            done++;
        }
        // cout << "\rEdges generated: " << done << "/" << edges << flush;
    }
    // cout << "Generated graph with " << nodes << " nodes and " << done << " edges.\n";
}

void break_edges(){
    //break edges
    cout << nodes;
    for (int i = 0; i < nodes; i++){
        int till = nodes - i;
        matrix[i][0] = nodes;
        for (int j = 1; j < till; j++){
            if (matrix[i][j] > -1) matrix[i][j] = -2;
        }
    }
    done = (nodes*(nodes-1))/2;

    while(done > edges){
        int white = rand() % nodes;
        int red;
        do {
            red = rand() % nodes;
        } while (red == white);

        if (white < red) {
            if (matrix[white][red - white] == -2){
                matrix[white][red - white] = 0;
                matrix[white][0]--;
                matrix[red][0]--;
                done--;
            }
        } else {
            if (matrix[red][white - red] == -2){
                matrix[red][white - red] = 0;
                matrix[white][0]--;
                matrix[red][0]--;
                done--;
            }
        }
    }

}

void fill_in_complete(){
    mt19937 rng(seed);
    int * used = new int[nodes]();
    uniform_int_distribution<int> dist(1, max_weight);
    for (int i = 0; i < nodes; i++){
        int till = nodes - i;
        // edge weights distinct and from 1 to nodes
        for (int j = 1; j < till; j++){
          matrix[i][j] = dist(rng);
        }
    }
    done = (nodes*(nodes-1))/2;
    return;
}

void fill_in(){
    for (int i = 0; i < nodes; i++){
        int till = nodes - i;
        for (int j = 1; j < till; j++){
            if (matrix[i][j] != 0){
                matrix[i][j] = rand() % max_weight + 1;
            }
        }
    }
    return;
}


void get_sizes(){
    // sizes is the number of  non-consecutive edges for each node
    // reseting when a zero is found
    edgesize = new int[nodes]();
    for (int i = 0; i < nodes; i++){
        for (int j = 1; j < nodes - i ; j++){
            if (matrix[i][j] > 0){
                edgesize[i]++;
                edgesize[j + i]++;
                if (last_index[i] + 1 == j + i){
                    // consecutive
                    last_index[i] = j + i;
                }
                else {
                    // cout << "consecutive edge for node " << i << " at " << j + i << "\n";
                    sizes[i]++;
                    last_index[i] = j + i;
                }
                if (last_index[j + i] + 1 == i ){
                    // consecutive
                    last_index[j + i] = i;
                }
                else {
                    // cout << "consecutive edge for node " << j + i << " at " << i << "\n";
                    // cout << "last index was " << last_index[j + i] << "\n";
                    sizes[j + i]++;
                    last_index[j + i] = i;
                }
            }
        }
    }
}

void print_it(){
    // file in input folder with name nodes_edges_seed_connectedcompleteregular.txt
    string filename = "input/making.txt";
      FILE* file = fopen(filename.c_str(), "wb");
    if (!file) return;
    
    // Use a large buffer for batch writing
    char buffer[65536];
    char* buf_ptr = buffer;
    const char* buf_end = buffer + sizeof(buffer) - 100; // Leave space for last writ
    
    // first line is edges(sizes) of each node
    for (int i = 0; i < nodes; i++){
        buf_ptr += sprintf(buf_ptr, "%d(%d) ", edgesize[i], sizes[i]);
        if (buf_ptr >= buf_end) {
            fwrite(buffer, 1, buf_ptr - buffer, file);
            buf_ptr = buffer;
        }
    }
    buf_ptr += sprintf(buf_ptr, "\n");

    
    // then the adjacency list
    int edge_number = 1;
    for (int i = 0; i < nodes; i++){

        for (int j = i+1; j < nodes; j++){
            if (matrix[i][j - i] != 0){
                //  cout << i <<  " " << j << " " << matrix[i][j-i] << endl;
                // Fast integer to string conversion and formatting
                buf_ptr += sprintf(buf_ptr, "%d(%d) ", j, matrix[i][j - i]);
                
                // Flush buffer when nearly full
                if (buf_ptr >= buf_end) {
                    fwrite(buffer, 1, buf_ptr - buffer, file);
                    buf_ptr = buffer;
                }
            }
        }
        buf_ptr += sprintf(buf_ptr, "\n");
    }
    
    // Write remaining data
    if (buf_ptr > buffer) {
        fwrite(buffer, 1, buf_ptr - buffer, file);
    }
    
    fclose(file);
    string filename_complete = "input/" + to_string(nodes) + "_" + to_string(edges) + "_" + to_string(seed) + "_" + to_string(connected) + to_string(complete) + to_string(regular) + ".txt";
    rename("input/making.txt", filename_complete.c_str());
    cout << "Graph written to file \033[1m" << filename_complete << "\033[0m\n";
    
    return;
}

void make_beeg(){
    string filename_complete = "input/" + to_string(nodes) + "_" + to_string(edges) + "_" + to_string(seed) + "_" + to_string(connected) + to_string(complete) + to_string(regular) + ".txt";
    FILE* file = fopen(filename_complete.c_str(), "wb");
    if (!file) return;
    // make as the smaller format directly
    // make each line then print it
    //first line is     nodes-1(1) 
    char buffer[65536];
    char* buf_ptr = buffer;;
    const char* buf_end = buffer + sizeof(buffer) - 100; // Leave space for last writ
    for (int i = 0 ; i < nodes; i++){
        buf_ptr += sprintf(buf_ptr, "%ld(1) ", nodes - 1);
        if (buf_ptr >= buf_end) {
            fwrite(buffer, 1, buf_ptr - buffer, file);
            buf_ptr = buffer;
        }
    }
    mt19937 rng(seed);
    // nextlines are adjacency list
    // cout << "required space: " << (double)(nodes * (nodes - 1) / 2 * sizeof(int) ) / (1024 * 1024 * 1024) << " GB\n";
    // cchec k if enough space is available
    
    buf_ptr += sprintf(buf_ptr, "\n");
    uniform_int_distribution<int> dist(1, max_weight);
    // int * used = new int[nodes]();
    for (int i = 0 ; i < nodes; i++){
        for (int j = i+1 ; j < nodes; j++){
            if (i != j){
                buf_ptr += sprintf(buf_ptr, "%d(%d) ", j, dist(rng));
                if (buf_ptr >= buf_end) {
                    fwrite(buffer, 1, buf_ptr - buffer, file);
                    buf_ptr = buffer;
                }
            }
        }
        buf_ptr += sprintf(buf_ptr, "\n");
        if (i % 1000 == 0) {
            cout << "\rWritten " << i << "/" << nodes << " nodes." << flush;
        }
    }
    // Write remaining data
    if (buf_ptr > buffer) {
        fwrite(buffer, 1, buf_ptr - buffer, file);
    }

    cout << "Graph written to file \033[1m" << filename_complete << "\033[0m\n";
  

    fclose(file);
    return;
}

void write_as_matrix(){
    string filename_matrix = "matrix.txt";
    FILE* file = fopen(filename_matrix.c_str(), "wb");
    if (!file) return;

    // Use a large buffer for batch writing
    char buffer[65536];
    char* buf_ptr = buffer;
    const char* buf_end = buffer + sizeof(buffer) - 100; // Leave space for last writ

    for (int i = 0; i < nodes; i++){
        for (int j = 0; j < nodes; j++){
            int value;
            if (i < j){
                value = matrix[i][j - i];
            }
            else if (i == j){
                value = 0;
            }
            else {
                value = matrix[j][i - j];
            }
            buf_ptr += sprintf(buf_ptr, "%d ", value);
            if (buf_ptr >= buf_end) {
                fwrite(buffer, 1, buf_ptr - buffer, file);
                buf_ptr = buffer;
            }
        }
        buf_ptr += sprintf(buf_ptr, "\n");
    }

    // Write remaining data
    if (buf_ptr > buffer) {
        fwrite(buffer, 1, buf_ptr - buffer, file);
    }

    fclose(file);
    cout << "Adjacency matrix written to file \033[1m" << filename_matrix << "\033[0m\n";
    return;
}

void write_as_list(){
    string filename = "comparision/list.txt";
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cerr << "Unable to open file \033[1mlist.txt\033[0m for writing.\n";
        return;
    }
    else cout << "Writing adjacency list to file \033[1mlist.txt\033[0m ...\n";
    outfile << nodes << " " << edges << "\n";
    long long total_weight = 0;
    for (int i = 0; i < nodes; i++){
        for (int j = i + 1; j < nodes; j++){
            int weight;
            if (i < j){
                weight = matrix[i][j - i];
            }
            else {
                weight = matrix[j][i - j];
            }
            if (weight != 0){
                outfile << i << " " << j << " " << weight << "\n";
                total_weight += weight;
            }
        }
    }
}

int main(int argc, char** argv){
    ifstream infile;

    infile = ifstream("input_params.json");
    if (!infile) {
        cerr << "Unable to open file input_params.json\n";
        exit(1); // terminate with error
    }

    while (!infile.eof()) {
        string key;
        char colon;
        if (infile >> key) {
            // cout << key << endl;
            success = 0;
            if (key == "\"seed\":") infile >> colon >> seed;
            else if (key == "\"nodes\":") infile >> colon  >> nodes;
            else if (key == "\"edges\":") infile >> colon  >> edges;
            else if (key == "\"connected\":"){ infile  >> key; connected = (key == "true" || key == "true,"); }
            else if (key == "\"complete\":") { infile  >> key; complete = (key == "true" || key == "true,"); }
            else if (key == "\"regular\":") { infile  >> key; regular = (key == "true" || key == "true,"); }
        }
    }

    // cout << "Parameters read from input_params.json:" << endl;
    
    density = (double)edges / ((nodes*(nodes-1))/2);
    if (nodes < 2) nodes = 2;
    if (edges > ((nodes)*(nodes-1))/2 ){ edges = ((nodes-1)*(nodes))/2; cout << "Edges exceed max possible edges, setting to complete graph.\n"; }
    if (edges < 0) edges = 0;
    if (complete) edges = ((nodes-1)*(nodes))/2;
    if (connected && edges < nodes - 1){
        edges = nodes - 1;
        // cout << "Edges less than minimum for connected graph, setting edges to " << edges << ".\n";
    }
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

    if (max_weight > nodes) max_weight = nodes;
    // if (nodes > 8000)max_weight = nodes / 10;
    
    
    
    cout << success << " " << seed << " " << nodes << " " << edges << " " << density << " "
    << connected << " " << complete << " " << regular << endl;

    string fine_pre_check = "input/" + to_string(nodes) + "_" + to_string(edges) + "_" + to_string(seed) + "_" + to_string(connected) + to_string(complete) + to_string(regular) + ".txt";
    ifstream infile_check(fine_pre_check);
    cout << "checking for file " << fine_pre_check << "\n";
    if (infile_check.good()){
        cout << "Graph with these parameters already exists as \033[1m" << fine_pre_check << "\033[0m\n";
        return 0;
    }
    // set the seed
    srand(seed);

    cout << "Space required by matrix during processing > " << (double)((nodes * (nodes - 1) / 2) * sizeof(int) ) / (1024 * 1024 * 1024) << " GB\n";
    
    if (nodes >= 8000 && complete == 1) {
        make_beeg();
        return 0;
    }

    matrix = new int32_t*[nodes]();
    meta = new int [nodes]();
    sizes = new int [nodes]();
    last_index = new int [nodes]();

    for (int i = 0; i < nodes; i++) {
        last_index[i] = -1;
        matrix[i] = new int32_t[nodes - i  ]();
    }

    // //print total space used
    // long long total_space = 0;
    // total_space += sizeof(int32_t*) * nodes; // matrix pointers
    // total_space += sizeof(int32_t) * (nodes * (nodes + 1)) / 2; // matrix data
    // total_space += sizeof(int) * nodes * 3; // meta, sizes, last_index
    // cout << "Total space allocated: " << (double)total_space / (1024 * 1024 * 1024) << " GB\n";

    if (regular != 1){
        //if connected make spanning tree first
        if (connected == 1 && complete == 0){
            spanit();
        }

        if (density < 0.5) {
            gen_edge();
        }
        else {
            break_edges();
        }
    }

    if (regular == 1 ){
        make_regular();
    }
    
    // gen_edge();
    
    if (complete == 1){
        edges = (nodes*(nodes-1))/2;
        fill_in_complete();
    }
    else fill_in();


    
    get_sizes();
    
    // print_graph();
    if (argc == 2) write_as_matrix();
    if (argc == 3) write_as_list();

    print_it();

    for (int i = 0; i < nodes; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;
    delete[] meta;
    delete[] sizes;
    delete[] last_index;
    delete[] edgesize;

}
