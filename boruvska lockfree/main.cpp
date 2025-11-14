#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
#include <queue>
#include <iomanip>
#include <sstream>

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

// Thread-safe logger
class Logger {
private:
    ofstream logFile;
    mutex logMutex;
    
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
        lock_guard<mutex> lock(logMutex);
        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        
        logFile << "[" << this_thread::get_id() << "] " 
                << message << endl;
        logFile.flush();
    }
};

// Union-Find (Disjoint Set Union) with path compression
class UnionFind {
private:
    vector<int> parent;
    vector<int> rank;
    mutex ufMutex;
    
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
        lock_guard<mutex> lock(ufMutex);
        
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

// Global logger instance
Logger* globalLogger = nullptr;

// Parallel edge extraction from adjacency matrix
void extractEdgesParallel(const vector<vector<double>>& adjMatrix, 
                          vector<Edge>& edges, 
                          int startRow, int endRow, 
                          mutex& edgesMutex) {
    
    ostringstream oss;
    oss << "Thread started: extracting edges from rows " << startRow << " to " << endRow;
    globalLogger->log(oss.str());
    
    vector<Edge> localEdges;
    int n = adjMatrix.size();
    
    for (int i = startRow; i < endRow; i++) {
        for (int j = i + 1; j < n; j++) {
            if (adjMatrix[i][j] > 0) {
                localEdges.push_back({i, j, adjMatrix[i][j]});
            }
        }
    }
    
    // Lock and merge local edges into global edges
    {
        lock_guard<mutex> lock(edgesMutex);
        edges.insert(edges.end(), localEdges.begin(), localEdges.end());
    }
    
    oss.str("");
    oss << "Thread finished: extracted " << localEdges.size() << " edges";
    globalLogger->log(oss.str());
}

// Read graph from input file
bool readGraph(const string& filename, vector<vector<double>>& adjMatrix) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Error opening input file!" << endl;
        return false;
    }
    
    int n;
    inFile >> n;
    
    adjMatrix.resize(n, vector<double>(n));
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            inFile >> adjMatrix[i][j];
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

// Kruskal's algorithm with parallel edge extraction and sorting
vector<Edge> kruskalMST(const vector<vector<double>>& adjMatrix, double& totalWeight) {
    int n = adjMatrix.size();
    vector<Edge> edges;
    vector<Edge> mst;
    mutex edgesMutex;
    
    globalLogger->log("=== Starting MST Computation ===");
    
    // Step 1: Parallel edge extraction
    globalLogger->log("Phase 1: Parallel edge extraction");
    
    int numThreads = thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;
    
    vector<thread> threads;
    int rowsPerThread = n / numThreads;
    
    for (int i = 0; i < numThreads; i++) {
        int startRow = i * rowsPerThread;
        int endRow = (i == numThreads - 1) ? n : (i + 1) * rowsPerThread;
        
        threads.emplace_back(extractEdgesParallel, ref(adjMatrix), 
                            ref(edges), startRow, endRow, ref(edgesMutex));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    ostringstream oss;
    oss << "Total edges extracted: " << edges.size();
    globalLogger->log(oss.str());
    
    // Step 2: Sort edges (using parallel sort if available)
    globalLogger->log("Phase 2: Sorting edges by weight");
    auto sortStart = high_resolution_clock::now();
    
    sort(edges.begin(), edges.end());
    
    auto sortEnd = high_resolution_clock::now();
    auto sortDuration = duration_cast<microseconds>(sortEnd - sortStart);
    
    oss.str("");
    oss << "Sorting completed in " << sortDuration.count() / 1000.0 << " ms";
    globalLogger->log(oss.str());
    
    // Step 3: Kruskal's algorithm with Union-Find
    globalLogger->log("Phase 3: Building MST with Kruskal's algorithm");
    
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
            globalLogger->log(oss.str());
            
            if (edgesAdded == n - 1) {
                break; // MST complete
            }
        }
    }
    
    globalLogger->log("=== MST Computation Complete ===");
    
    return mst;
}

int main() {
    // Initialize logger
    auto mstStart = high_resolution_clock::now();
    Logger logger("log.txt");
    globalLogger = &logger;
    
    logger.log("========================================");
    logger.log("Parallel MST Calculator Started");
    logger.log("========================================");
    
    auto programStart = high_resolution_clock::now();
    
    // Read input graph
    logger.log("Reading input graph from input.txt");
    vector<vector<double>> adjMatrix;
    
    if (!readGraph("input.txt", adjMatrix)) {
        logger.log("ERROR: Failed to read input file");
        return 1;
    }
    
    ostringstream oss;
    oss << "Graph loaded: " << adjMatrix.size() << " vertices";
    logger.log(oss.str());
    
    // Compute MST
    
    double totalWeight = 0;
    vector<Edge> mst = kruskalMST(adjMatrix, totalWeight);
    
    auto mstEnd = high_resolution_clock::now();
    auto mstDuration = duration_cast<milliseconds>(mstEnd - mstStart);
    
    // Write output
    logger.log("Writing MST to output.txt");
    writeMST("output.txt", mst, totalWeight);
    
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
    // cout << "Results written to output.txt" << endl;
    // cout << "Logs written to log.txt" << endl;
    
    return 0;
}