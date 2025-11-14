#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <limits>
#include <iomanip>
#include <map>
#include<climits>
using namespace std;
using namespace chrono;

struct Edge {
    int u, v, weight;
    Edge(int u = 0, int v = 0, int w = 0) : u(u), v(v), weight(w) {}
    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }
};

class UnionFind {
private:
    vector<int> parent, rank;
    mutex uf_mutex;
    
public:
    UnionFind(int n) : parent(n), rank(n, 0) {
        for (int i = 0; i < n; i++) parent[i] = i;
    }
    
    int find(int x) {
        if (parent[x] != x)
            parent[x] = find(parent[x]);
        return parent[x];
    }
    
    bool unite(int x, int y) {
        // lock_guard<mutex> lock(uf_mutex);
        int px = find(x), py = find(y);
        if (px == py) return false;
        
        if (rank[px] < rank[py]) swap(px, py);
        parent[py] = px;
        if (rank[px] == rank[py]) rank[px]++;
        return true;
    }
};

class ParallelMST {
private:
    int n;
    vector<vector<int>> adj_matrix;
    vector<Edge> mst_edges;
    ofstream log_file;
    mutex log_mutex, mst_mutex;
    int num_threads;
    map<thread::id, int> thread_id_map;
    
    void log(const string& msg) { 
        // return;
        lock_guard<mutex> lock(log_mutex);
        int thread_num = thread_id_map[this_thread::get_id()];
        log_file << "[Thread " << thread_num << "] " << msg << endl;
    }
    
    void find_min_edges_parallel(UnionFind& uf, vector<Edge>& edges, 
                                 vector<Edge>& min_edges, int start, int end) {
        log("Thread started processing components " + to_string(start) + 
            " to " + to_string(end));
        
        for (int comp = start; comp < end; comp++) {
            Edge min_edge(-1, -1, INT_MAX);
            
            // Find minimum edge for this component
            for (const auto& e : edges) {
                int comp_u = uf.find(e.u);
                int comp_v = uf.find(e.v);
                
                if (comp_u == comp && comp_v != comp) {
                    if (e.weight < min_edge.weight) {
                        min_edge = e;
                    }
                } else if (comp_v == comp && comp_u != comp) {
                    if (e.weight < min_edge.weight) {
                        min_edge = e;
                    }
                }
            }
            
            if (min_edge.u != -1) {
                min_edges[comp] = min_edge;
            }
        }
        
        log("Thread finished processing components");
    }
    
public:
    ParallelMST(const string& input_file, int threads = 4) : num_threads(threads) {
        log_file.open("mst_log.txt");
        log_file << "=== Parallel MST Generation Log ===" << endl;
        log_file << "Timestamp: " << system_clock::to_time_t(system_clock::now()) << endl;
        log_file << "Number of threads: " << num_threads << endl << endl;
        
        // Read adjacency matrix
        ifstream fin(input_file);
        if (!fin) {
            cerr << "Error opening input file!" << endl;
            exit(1);
        }
        
        fin >> n;
        adj_matrix.resize(n, vector<int>(n));
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                fin >> adj_matrix[i][j];
            }
        }
        fin.close();
        
        log("Loaded graph with " + to_string(n) + " vertices");
    }
    
    void compute_mst() {
        
        // Register main thread
        thread_id_map[this_thread::get_id()] = 0;
        
        log("Starting Boruvka's MST algorithm (parallelized)");
        
        // Create edge list from adjacency matrix
        vector<Edge> edges;
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                if (adj_matrix[i][j] > 0) {
                    edges.push_back(Edge(i, j, adj_matrix[i][j]));
                }
            }
        }
        
        log("Total edges in graph: " + to_string(edges.size()));
        
        auto start_time = high_resolution_clock::now();
        UnionFind uf(n);
        int num_components = n;
        int iteration = 0;
        
        while (num_components > 1) {
            iteration++;
            log("\n=== Iteration " + to_string(iteration) + " ===");
            log("Current components: " + to_string(num_components));
            
            vector<Edge> min_edges(n, Edge(-1, -1, INT_MAX));
            
            // Parallel phase: Find minimum edge for each component
            vector<thread> threads;
            int chunk_size = (n + num_threads - 1) / num_threads;
            
            for (int t = 0; t < num_threads; t++) {
                int start = t * chunk_size;
                int end = min(start + chunk_size, n);
                if (start < n) {
                    threads.emplace_back([this, t, &uf, &edges, &min_edges, start, end]() {
                        // Register thread ID
                        {
                            // lock_guard<mutex> lock(log_mutex);
                            thread_id_map[this_thread::get_id()] = t + 1;
                        }
                        this->find_min_edges_parallel(uf, edges, min_edges, start, end);
                    });
                }
            }
            
            for (auto& th : threads) {
                th.join();
            }
            
            log("All threads completed finding minimum edges");
            
            // Add edges to MST and merge components
            int edges_added = 0;
            for (int i = 0; i < n; i++) {
                if (min_edges[i].u != -1) {
                    int comp_u = uf.find(min_edges[i].u);
                    int comp_v = uf.find(min_edges[i].v);
                    
                    if (comp_u != comp_v) {
                        if (uf.unite(comp_u, comp_v)) {
                            // lock_guard<mutex> lock(mst_mutex);
                            mst_edges.push_back(min_edges[i]);
                            edges_added++;
                            num_components--;
                            
                            log("Added edge (" + to_string(min_edges[i].u) + 
                                ", " + to_string(min_edges[i].v) + 
                                ") with weight " + to_string(min_edges[i].weight));
                        }
                    }
                }
            }
            
            log("Edges added in this iteration: " + to_string(edges_added));
            
            if (edges_added == 0 && num_components > 1) {
                log("WARNING: No edges added but components remain - graph may be disconnected");
                break;
            }
        }
        
        auto end_time = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end_time - start_time);
        
        log("\n=== MST Computation Complete ===");
        log("Total edges in MST: " + to_string(mst_edges.size()));
        log("Time taken: " + to_string(duration.count()) + " ms");
        
        cout << "MST generation time: " << duration.count() << " ms" << endl;
    }
    
    void write_output() {
        // Sort edges by weight
        sort(mst_edges.begin(), mst_edges.end());
        
        ofstream fout("mst_output.txt");
        fout << "=== Minimum Spanning Tree ===" << endl;
        fout << "Total edges: " << mst_edges.size() << endl;
        fout << "Format: (u, v) weight" << endl << endl;
        
        int total_weight = 0;
        for (const auto& e : mst_edges) {
            fout << "(" << e.u << ", " << e.v << ") " << e.weight << endl;
            total_weight += e.weight;
        }
        
        fout << "\nTotal MST weight: " << total_weight << endl;
        fout.close();
        
        log("\nOutput written to mst_output.txt");
        // cout << "Output written to mst_output.txt" << endl;
        // cout << "Log written to mst_log.txt" << endl;
    }
    
    ~ParallelMST() {
        log_file.close();
    }
};

int main(int argc, char* argv[]) {
    string input_file = "graph.txt";
    int num_threads = thread::hardware_concurrency();
    
    if (argc > 1) input_file = argv[1];
    if (argc > 2) num_threads = atoi(argv[2]);
    
    // cout << "Parallel MST Generator using Boruvka's Algorithm" << endl;
    // cout << "Using " << num_threads << " threads" << endl;
    // cout << "Input file: " << input_file << endl << endl;
    
    ParallelMST mst(input_file, num_threads);
    mst.compute_mst();
    mst.write_output();
    
    return 0;
}