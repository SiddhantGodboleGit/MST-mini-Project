import numpy as np

def generate_random_graph_files(n, isConnected=0, num_edges=None, max_weight=1400000,
                                matrix_filename="input.txt", list_filename="input_list.txt"):
    
    max_possible_edges = n * (n - 1) // 2
    min_edges = n - 1 if isConnected == 1 else 0

    # Validate num_edges
    if num_edges is None:
        num_edges = min_edges if isConnected == 1 else int(max_possible_edges * 0.5)
    num_edges = max(min_edges, min(num_edges, max_possible_edges))

    
    edges_dict = {}
    adjacency_list = {i: [] for i in range(n)}

    if isConnected == 1:
        # Step 1: Create spanning tree (n-1 edges)
        vertices = list(range(n))
        np.random.shuffle(vertices)
        for i in range(1, n):
            u, v = vertices[i], vertices[np.random.randint(0, i)]
            weight = np.random.randint(1, max_weight + 1)
            edge_key = (min(u, v), max(u, v))
            edges_dict[edge_key] = weight
            adjacency_list[u].append((v, weight))
            adjacency_list[v].append((u, weight))

        # Step 2: Add remaining edges
        remaining_edges = num_edges - (n - 1)
        if remaining_edges > 0:
            # Generate possible edges on-the-fly to avoid storing large list
            added = 0
            attempts = 0
            max_attempts = remaining_edges * 10  # Prevent infinite loop
            
            while added < remaining_edges and attempts < max_attempts:
                u = np.random.randint(0, n)
                v = np.random.randint(0, n)
                if u != v:
                    edge_key = (min(u, v), max(u, v))
                    if edge_key not in edges_dict:
                        weight = np.random.randint(1, max_weight + 1)
                        edges_dict[edge_key] = weight
                        adjacency_list[u].append((v, weight))
                        adjacency_list[v].append((u, weight))
                        added += 1
                attempts += 1

    else:  # disconnected graph
        # For disconnected graphs, randomly generate edges
        added = 0
        attempts = 0
        max_attempts = num_edges * 10
        
        while added < num_edges and attempts < max_attempts:
            u = np.random.randint(0, n)
            v = np.random.randint(0, n)
            if u != v:
                edge_key = (min(u, v), max(u, v))
                if edge_key not in edges_dict:
                    weight = np.random.randint(1, max_weight + 1)
                    edges_dict[edge_key] = weight
                    adjacency_list[u].append((v, weight))
                    adjacency_list[v].append((u, weight))
                    added += 1
            attempts += 1

    # Write adjacency list file first (uses existing data structure)
    with open(list_filename, "w") as f:
        f.write(f"{n}\n")
        for i in range(n):
            neighbors = " ".join(f"{v}({w})" for v, w in adjacency_list[i])
            f.write(f"{i}: {neighbors}\n")

    # Write adjacency matrix file row-by-row to avoid storing full matrix
    with open(matrix_filename, "w") as f:
        f.write(f"{n}\n")
        for i in range(n):
            row = []
            for j in range(n):
                if i == j:
                    row.append(0)
                else:
                    edge_key = (min(i, j), max(i, j))
                    row.append(edges_dict.get(edge_key, 0))
            f.write(" ".join(map(str, row)) + "\n")

    # Clean up to free memory
    del edges_dict
    del adjacency_list


if __name__ == "__main__":
    n = int(input("Number of vertices: "))
    connected = int(input("Should the graph be connected? (1 = Yes, 0 = No): "))
    num_edges = int(input("Number of edges: "))

    generate_random_graph_files(n, connected, num_edges)