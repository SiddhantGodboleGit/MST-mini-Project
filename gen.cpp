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

using namespace std;

int ** matrix;
int *meta;
int success, seed, nodes, edges;
double density;
bool connected, complete, regular;

int done = 0;

void spanit(){
    //make a random spanning tree
    //choose a node at random
    //choose another node at random and connect it to one of the connected nodes
    vector<int> connected;
    vector<int> unconnected;
    int connected_nodes = 1;
    int unconnected_nodes = nodes - 1;

    for (int i = 0; i < nodes; i++){
        unconnected.push_back(i);
    }

    int now = rand() % nodes;

    connected.push_back(unconnected[now]);
    unconnected[now] = unconnected[unconnected_nodes];
    unconnected_nodes--;

    while (connected_nodes < nodes){
        int white = unconnected[rand() % unconnected_nodes];
        int red;
        do {
            red = connected[rand() % connected_nodes];
        } while (red == white);

        if (white < red) {
            matrix[white][red - white] = -1;
        } else {
            matrix[red][white - red] = -1;
        }
        matrix[white][0]++;
        matrix[red][0]++;
        done++;

        connected.push_back(unconnected[white]);
        unconnected[white] = unconnected[unconnected_nodes];
        unconnected_nodes--;
        connected_nodes++;
    }
    return;
}

void gen_edge(){
    //generate edges
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

        if (matrix[red][white - red] == 0){
            matrix[red][white - red] = -2;
            matrix[white][0]++;
            matrix[red][0]++;
            done++;
        }
        
    }
}

void break_edges(){
    //break edges
    for (int i = 0; i < nodes; i++){
        int till = nodes - i;
        matrix[i][0] = nodes;
        for (int j = 0; j < till; j++){
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
    for (int i = 0; i < nodes; i++){
        int till = nodes - i;
        matrix[i][0] = nodes - 1;
        int max_weight = nodes * 2;
        // edge weights distinct and from 1 to nodes
        for (int j = 1; j < till; j++){
          matrix[i][j] = rand() % max_weight + 1;
        }
    }
    done = (nodes*(nodes-1))/2;
    return;
}

void fill_in(){
    for (int i = 0; i < nodes; i++){
        int till = nodes - i;
        int max_weight = nodes * 2;
        for (int j = 1; j < till; j++){
            if (matrix[i][j] != 0){
                matrix[i][j] = rand() % max_weight + 1;
            }
        }
    }
    return;
}


void print_it(){
    // file in input folder with name nodes_edges_seed_connectedcompleteregular.txt
    string filename = "input/making.txt";
    //empty the file first
    ofstream outfile;
    outfile.open(filename , std::ios::out | std::ios::trunc);
    // outfile << nodes << " " << done << "\n";
    int total = 0;
    for (int i = 0; i < nodes; i++){
        matrix[i][0] = -1 * matrix[i][0];
        for (int j = i+1; j < nodes; j++){
            if (matrix[i][j - i] != 0 ){
                outfile << i << " " << j << " " << matrix[i][j - i] << "\n";
                total++;
            }
        }
    }
    cout << "Generated graph with " << nodes << " nodes and " << total << " edges.\n";
    string filename_complete = "input/" + to_string(nodes) + "_" + to_string(edges) + "_" + to_string(seed) + "_" + to_string(connected) + to_string(complete) + to_string(regular) + ".txt";
    outfile.close();
    rename("input/making.txt", filename_complete.c_str());
    return;
}

int main() {
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
            else if (key == "\"connected\":"){ infile >> colon  >> key; connected = (key == "true" || key == "true,"); }
            else if (key == "\"complete\":") { infile >> colon  >> key; complete = (key == "true" || key == "true,"); }
            else if (key == "\"regular\":") { infile >> colon  >> key; regular = (key == "true" || key == "true,"); }
        }
    }
    // cout << "Parameters read from input_params.json:" << endl;
    // cout << success << " " << seed << " " << nodes << " " << edges << " " << density << " "
    //      << connected << " " << complete << " " << regular << endl;
    
    density = (double)edges / ((nodes*(nodes-1))/2);
    if (nodes < 2) nodes = 2;
    if (edges > ((nodes)*(nodes-1))/2 ) edges = ((nodes-1)*(nodes))/2;
    if (edges < 0) edges = 0;
    
    // set the seed
    srand(seed);

    matrix = new int*[nodes];
    meta = new int [nodes];
    for (int i = 0; i < nodes; i++) {
        matrix[i] = new int[nodes - i ]();
    }

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

    if (complete == 1){
        fill_in_complete();
    }
    else fill_in();

    print_it();

    for (int i = 0; i < nodes; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;

}
