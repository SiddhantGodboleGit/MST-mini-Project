import numpy as np

def generate_random_graph_files(n, isConnected=0, num_edges=None, max_weight=140,
                                matrix_filename="input.txt", list_filename="input_list.txt"):
    """
    Generate a random undirected weighted graph and save both
    adjacency matrix and adjacency list representations to files.

    Parameters:
        n (int): Number of vertices
        isConnected (int): 1 -> connected, 0 -> disconnected
        num_edges (int): Desired number of edges (optional)
        max_weight (int): Maximum edge weight
        matrix_filename (str): Output file for adjacency matrix
        list_filename (str): Output file for adjacency list
    """
    max_possible_edges = n * (n - 1) // 2
    min_edges = n - 1 if isConnected == 1 else 0

    # Validate num_edges
    if num_edges is None:
        num_edges = min_edges if isConnected == 1 else int(max_possible_edges * 0.5)
    num_edges = max(min_edges, min(num_edges, max_possible_edges))

    # Initialize structures
    A = np.zeros((n, n), dtype=int)
    adjacency_list = {i: [] for i in range(n)}
    edges = set()

    if isConnected == 1:
        # Step 1: Create spanning tree (n-1 edges)
        vertices = list(range(n))
        np.random.shuffle(vertices)
        for i in range(1, n):
            u, v = vertices[i], vertices[np.random.randint(0, i)]
            weight = np.random.randint(1, max_weight + 1)
            A[u, v] = A[v, u] = weight
            adjacency_list[u].append((v, weight))
            adjacency_list[v].append((u, weight))
            edges.add(tuple(sorted((u, v))))

        # Step 2: Add remaining edges
        remaining_edges = num_edges - (n - 1)
        possible_edges = [(i, j) for i in range(n) for j in range(i + 1, n) if (i, j) not in edges]
        np.random.shuffle(possible_edges)

        for (u, v) in possible_edges[:remaining_edges]:
            weight = np.random.randint(1, max_weight + 1)
            A[u, v] = A[v, u] = weight
            adjacency_list[u].append((v, weight))
            adjacency_list[v].append((u, weight))
            edges.add((u, v))

    else:  # disconnected graph
        possible_edges = [(i, j) for i in range(n) for j in range(i + 1, n)]
        np.random.shuffle(possible_edges)
        for (u, v) in possible_edges[:num_edges]:
            weight = np.random.randint(1, max_weight + 1)
            A[u, v] = A[v, u] = weight
            adjacency_list[u].append((v, weight))
            adjacency_list[v].append((u, weight))

    # Write adjacency matrix file
    with open(matrix_filename, "w") as f:
        f.write(f"{n}\n")
        for row in A:
            f.write(" ".join(map(str, row)) + "\n")

    # Write adjacency list file
    # with open(list_filename, "w") as f:
    #     f.write(f"{n}\n")
    #     for i in range(n):
    #         neighbors = " ".join(f"{v}({w})" for v, w in adjacency_list[i])
    #         f.write(f"{i}: {neighbors}\n")

    # print(f"✅ Graph generated with {n} vertices and {len(edges)} edges.")
    # print(f"   Matrix saved to: {matrix_filename}")
    # print(f"   List saved to:   {list_filename}")


if __name__ == "__main__":
    n = int(input("Number of vertices: "))
    connected = int(input("Should the graph be connected? (1 = Yes, 0 = No): "))
    num_edges = int(input("Number of edges: "))

    generate_random_graph_files(n, connected, num_edges)
