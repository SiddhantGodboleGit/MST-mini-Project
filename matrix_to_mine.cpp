#include <iostream>
#include <fstream>
#include <string>
// 
using namespace std;
// program to convert a adjecency matrix format to my format 
// read matrix.txt and output to input_params.json and make a input/.....txt file

int main(){
    string filename_matrix = "matrix.txt";
    ifstream infile;
    infile.open(filename_matrix);
    if (!infile.is_open()){
        cerr << "File \033[1m" << filename_matrix << "\033[0m does not exist. Please generate the graph first using 'make graph'.\n";
        return 1;
    }
    int n;
    cout << "Give number of nodes: ";
    cin >> n;
    int ** matrix = new int*[n];
    for (int i = 0; i < n; i++){
        matrix[i] = new int[n]();
    }
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            int weight;
            infile >> weight;
            matrix[i][j] = weight;
        }
    }
    infile.close();

    // write to file in my format
    FILE* file = fopen("input/making.txt", "wb");
    if (!file) return 1;

    int * edgesize ;
    int nodes = n;
    int edges = 0;
    int * sizes;
    // int * last_index;
    // last_index = new int[nodes]();
    sizes = new int[nodes]();
    int last_index = 0;

        edgesize = new int[nodes]();
        for (int i = 0; i < nodes; i++){
            last_index = -1;
            for (int j = 0; j < nodes; j++){
                if (matrix[i][j] > 0){
                    edgesize[i]++;
                    edges++;
                    // cout << "Edge found for node " << i << " at " << j << " with weight " << matrix[i][j] << "\n";
                    if (last_index + 1 == j){
                        // consecutive
                        last_index = j;
                    }
                    else {
                        // cout << "consecutive edge for node " << i << " at " << j << "\n";
                        sizes[i]++;
                        last_index = j;
                    }
                }
            }
        }
        edges = edges /2 ;
        // for (int i = 0; i < nodes; i++)
        // cout << "Node " << i << " has edgesize " << edgesize[i] << " and sizes " << sizes[i] << "\n";
    // make the output file
    char buffer[65536];
    char* buf_ptr = buffer;
    const char* buf_end = buffer + sizeof(buffer) - 100; // Leave space for last write

    // first line is edges(sizes) of each node
    for (int i = 0; i < nodes; i++){
        buf_ptr += sprintf(buf_ptr, "%d(%d) ", edgesize[i], sizes[i]);
        // cout << "Node " << i << " has edgesize " << edgesize[i] << " and sizes " << sizes[i] << "\n";
        if (buf_ptr >= buf_end) {
            fwrite(buffer, 1, buf_ptr - buffer, file);
            buf_ptr = buffer;
        }
    }
    buf_ptr += sprintf(buf_ptr, "\n");

    // // if (buf_ptr > buf_end) {
    //     fwrite(buffer, 1, buf_ptr - buffer, file);
    // //     buf_ptr = buffer;
    // // }

    // then the adjacency list
    for (int i = 0; i < nodes; i++){    
        for (int j = i+1; j < nodes; j++){
            if (matrix[i][j] != 0){
                // edges++;
                // Fast integer to string conversion and formatting
                buf_ptr += sprintf(buf_ptr, "%d(%d) ", j, matrix[i][j]);
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
    cout << "Graph has " << nodes << " nodes and " << edges << " edges.\n";
    // usleep(100000); // wait for output to flush
    fclose(file);
    string filename_params = "input_params.json";
    file = fopen(filename_params.c_str(), "wb");
    if (!file) return 1;    
    // write json file
    const char * json_start = "{\n\"nodes\": ";
    buf_ptr = buffer;
    buf_ptr += sprintf(buf_ptr, "%s\"%d\",\n\"edges\": \"%d\",\n\"threads\": \"%d\",\n", json_start, nodes, edges, 4);
    buf_ptr += sprintf(buf_ptr, "\"seed\": \"0\",\n");
    buf_ptr += sprintf(buf_ptr, "\"connected\": false,\n\"complete\": false,\n\"regular\": false\n}\n");
    fwrite(buffer, 1, buf_ptr - buffer, file);
    fclose(file);

    string filename_complete = "input/" + to_string(nodes) + "_" + to_string(edges) + "_0_000.txt";
    rename("input/making.txt", filename_complete.c_str());
    cout << "Graph written to file \033[1m" << filename_complete << "\033[0m\n";
    delete[] edgesize;
    delete[] sizes;
    // delete[] last_index;
    for (int i = 0; i < n; i++){
        delete[] matrix[i];
    }
    delete[] matrix;
    return 0;
}

