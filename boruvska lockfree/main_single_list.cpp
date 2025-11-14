#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;
using namespace chrono;

// Edge structure
struct Edge {
    int u, v;
    double weight;
    
    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }
};

// Simple logger for single-threaded execution
class Logger {
private:
    ofstream logFile;
    
public:
    Logger(const string& filename) {
        logFile.open(filename);
        if (!logFile.is_open()) {
            cerr << "Error opening log file!" << endl;
        }
    }
    
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    void log(const string& message) {
        logFile << message << endl;
        logFile.flush();
    }
};

// Union-Find (Disjoint Set Union) with path compression
class UnionFind {
private:
    vector<int> parent;
    vector<int> rank;
    
public:
    UnionFind(int n) : parent(n), rank(n, 0) {
        for (int i = 0; i < n; i++) {
            parent[i] = i;
        }
    }
    
    int find(int x) {
        if (parent[x] != x) {
            parent[x] = find(parent[x]); // Path compression
        }
        return parent[x];
    }
    
    bool unite(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);
        
        if (rootX == rootY) {
            return false; // Already in same set
        }
        
        // Union by rank
        if (rank[rootX] < rank[rootY]) {
            parent[rootX] = rootY;
        } else if (rank[rootX] > rank[rootY]) {
            parent[rootY] = rootX;
        } else {
            parent[rootY] = rootX;
            rank[rootX]++;
        }
        
        return true;
    }
};

// Adjacency list representation
struct AdjacencyList {
    int numVertices;
    vector<vector<pair<int, double>>> adjList; // adjList[u] = {(v, weight), ...}
};

// Extract edges from adjacency list
vector<Edge> extractEdges(const AdjacencyList& graph, Logger& logger) {
    logger.log("Phase 1: Extracting edges from adjacency list");
    
    vector<Edge> edges;
    
    for (int u = 0; u < graph.numVertices; u++) {
        for (const auto& neighbor : graph.adjList[u]) {
            int v = neighbor.first;
            double weight = neighbor.second;
            
            // Only add edge once (when u < v) to avoid duplicates
            if (u < v) {
                edges.push_back({u, v, weight});
            }
        }
    }
    
    ostringstream oss;
    oss << "Total edges extracted: " << edges.size();
    logger.log(oss.str());
    
    return edges;
}

// Read graph from input file (adjacency list format)
bool readGraph(const string& filename, AdjacencyList& graph) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Error opening input file!" << endl;
        return false;
    }
    
    // Read number of vertices
    inFile >> graph.numVertices;
    graph.adjList.resize(graph.numVertices);
    
    string line;
    getline(inFile, line); // Consume newline after number
    
    // Read adjacency list
    for (int i = 0; i < graph.numVertices; i++) {
        if (!getline(inFile, line)) {
            cerr << "Error: Not enough lines in input file!" << endl;
            return false;
        }
        
        // Parse line: "vertex: neighbor1(weight1) neighbor2(weight2) ..."
        istringstream iss(line);
        
        int vertex;
        char colon;
        iss >> vertex >> colon;
        
        if (vertex != i) {
            cerr << "Warning: Expected vertex " << i << " but found " << vertex << endl;
        }
        
        string neighborData;
        while (iss >> neighborData) {
            // Parse "neighbor(weight)"
            size_t openParen = neighborData.find('(');
            size_t closeParen = neighborData.find(')');
            
            if (openParen == string::npos || closeParen == string::npos) {
                cerr << "Error parsing neighbor data: " << neighborData << endl;
                continue;
            }
            
            int neighbor = stoi(neighborData.substr(0, openParen));
            double weight = stod(neighborData.substr(openParen + 1, closeParen - openParen - 1));
            
            graph.adjList[vertex].push_back({neighbor, weight});
        }
    }
    
    inFile.close();
    return true;
}

// Write MST to output file
void writeMST(const string& filename, const vector<Edge>& mst, double totalWeight) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "Error opening output file!" << endl;
        return;
    }
    
    outFile << "Minimum Spanning Tree Edges:" << endl;
    outFile << "----------------------------" << endl;
    
    for (const auto& edge : mst) {
        outFile << "Edge: " << edge.u << " - " << edge.v 
                << " | Weight: " << fixed << setprecision(2) << edge.weight << endl;
    }
    
    outFile << "----------------------------" << endl;
    outFile << "Total MST Weight: " << fixed << setprecision(2) << totalWeight << endl;
    
    outFile.close();
}

// Kruskal's algorithm for MST
vector<Edge> kruskalMST(const AdjacencyList& graph, double& totalWeight, Logger& logger) {
    int n = graph.numVertices;
    vector<Edge> mst;
    
    logger.log("=== Starting MST Computation ===");
    
    // Step 1: Extract edges
    vector<Edge> edges = extractEdges(graph, logger);
    
    // Step 2: Sort edges by weight
    logger.log("Phase 2: Sorting edges by weight");
    auto sortStart = high_resolution_clock::now();
    
    sort(edges.begin(), edges.end());
    
    auto sortEnd = high_resolution_clock::now();
    auto sortDuration = duration_cast<microseconds>(sortEnd - sortStart);
    
    ostringstream oss;
    oss << "Sorting completed in " << sortDuration.count() / 1000.0 << " ms";
    logger.log(oss.str());
    
    // Step 3: Build MST using Kruskal's algorithm with Union-Find
    logger.log("Phase 3: Building MST with Kruskal's algorithm");
    
    UnionFind uf(n);
    totalWeight = 0;
    int edgesAdded = 0;
    
    for (const auto& edge : edges) {
        if (uf.unite(edge.u, edge.v)) {
            mst.push_back(edge);
            totalWeight += edge.weight;
            edgesAdded++;
            
            oss.str("");
            oss << "MST Edge #" << edgesAdded << ": " << edge.u << "-" << edge.v 
                << " (weight: " << edge.weight << ") | Total weight so far: " << totalWeight;
            logger.log(oss.str());
            
            if (edgesAdded == n - 1) {
                break; // MST complete
            }
        }
    }
    
    logger.log("=== MST Computation Complete ===");
    
    return mst;
}

int main() {
    // Initialize logger
    auto mstStart = high_resolution_clock::now();
    Logger logger("log_single_list.txt");
    
    logger.log("========================================");
    logger.log("Single-Threaded MST Calculator (Adjacency List Version)");
    logger.log("========================================");
    
    auto programStart = high_resolution_clock::now();
    
    // Read input graph
    logger.log("Reading input graph from input_list.txt");
    AdjacencyList graph;
    
    if (!readGraph("input_list.txt", graph)) {
        logger.log("ERROR: Failed to read input file");
        return 1;
    }
    
    ostringstream oss;
    oss << "Graph loaded: " << graph.numVertices << " vertices";
    logger.log(oss.str());
    
    // Compute MST
    
    double totalWeight = 0;
    vector<Edge> mst = kruskalMST(graph, totalWeight, logger);
    
    auto mstEnd = high_resolution_clock::now();
    auto mstDuration = duration_cast<milliseconds>(mstEnd - mstStart);
    
    // Write output
    logger.log("Writing MST to output_single_list.txt");
    writeMST("output_single_list.txt", mst, totalWeight);
    
    auto programEnd = high_resolution_clock::now();
    auto totalDuration = duration_cast<milliseconds>(programEnd - programStart);
    
    // Log timing information
    logger.log("========================================");
    oss.str("");
    oss << "MST Computation Time: " << mstDuration.count() << " ms";
    logger.log(oss.str());
    
    oss.str("");
    oss << "Total Program Execution Time: " << totalDuration.count() << " ms";
    logger.log(oss.str());
    
    oss.str("");
    oss << "MST Total Weight: " << fixed << setprecision(2) << totalWeight;
    logger.log(oss.str());
    
    logger.log("========================================");
    logger.log("Program Completed Successfully");
    logger.log("========================================");
    
    // Also output to console
    // cout << "MST computed successfully!" << endl;
    // cout << "Total Weight: " << fixed << setprecision(2) << totalWeight << endl;
    cout << "Execution Time: " << mstDuration.count() << " ms" << endl;
    // cout << "Results written to output_single_list.txt" << endl;
    // cout << "Logs written to log_single_list.txt" << endl;
    
    return 0;
}