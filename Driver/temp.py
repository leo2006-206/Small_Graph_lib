import math
import random
from collections import defaultdict

class FastMeanStd:
    def __init__(self):
        self.count = 0
        self.sum = 0
        self.sum_sq = 0

    def add(self, var):
        self.count += 1
        self.sum += var
        self.sum_sq += var * var

    def get_mean(self):
        if self.count == 0: return 0
        return self.sum // self.count

    def get_std(self):
        if self.count == 0: return 0
        mean_f = self.sum / self.count
        mean_sq_f = self.sum_sq / self.count
        variance = mean_sq_f - (mean_f * mean_f)
        if variance < 0.0: variance = 0.0
        return int(math.sqrt(variance))

class CSRGraph:
    def __init__(self, num_nodes, edge_list):
        self.num_nodes = num_nodes
        self.node_vec = [0] * (num_nodes + 1)
        self.edge_vec = []
        
        adj = defaultdict(list)
        for u, v in edge_list:
            adj[u].append(v)
            
        current_offset = 0
        for i in range(num_nodes):
            self.node_vec[i] = current_offset
            # Standard CSR builders usually sort destinations to ensure deterministic layout
            adj[i].sort() 
            for v in adj[i]:
                self.edge_vec.append(v)
                current_offset += 1
        self.node_vec[num_nodes] = current_offset

def simulate_dfs_mem_distance(graph, start_id=0):
    visited = [False] * graph.num_nodes
    ms = FastMeanStd()
    
    is_first = True
    prev_source = -1
    prev_addr = -1
    
    # Exact replica of your C++ stack and visit logic
    stack = []
    
    if start_id < graph.num_nodes:
        stack.append(start_id)
        visited[start_id] = True
        
    print(f"\n--- Starting Iterative Stack DFS Trace (Start Node: {start_id}) ---")
    
    while len(stack) > 0:
        current_node = stack.pop()
        
        start_idx = graph.node_vec[current_node]
        end_idx = graph.node_vec[current_node + 1]
        
        for i in range(start_idx, end_idx):
            current_edge = graph.edge_vec[i]
            
            # NOTE: Assumes 4 bytes per edge (uint32_t). 
            # If your graph uses uint64_t, change this to: addr = i * 8
            addr = i * 4 
            
            # --- Functor Logic ---
            if is_first:
                print(f"Read Edge {current_node}->{current_edge} at Addr {addr:02d}. (Initial edge, no jump)")
                prev_source = current_node
                prev_addr = addr
                is_first = False
            else:
                if current_node != prev_source:
                    diff = abs(prev_addr - addr)
                    ms.add(diff)
                    print(f"Read Edge {current_node}->{current_edge} at Addr {addr:02d}. Jumped from node {prev_source}. Diff = {diff}")
                    prev_source = current_node
                    prev_addr = addr
                else:
                    print(f"Read Edge {current_node}->{current_edge} at Addr {addr:02d}. (Same node, no jump)")
                    prev_addr = addr
            
            # --- Visited & Stack Logic ---
            if not visited[current_edge]:
                visited[current_edge] = True
                stack.append(current_edge)

    print("\n--- Results ---")
    print(f"CSR edge_vec_: {graph.edge_vec}")
    print(f"Total Jumps Recorded: {ms.count}")
    print(f"Expected Mean: {ms.get_mean()}")
    print(f"Expected Std:  {ms.get_std()}\n")


def generate_and_test_random_graph(num_nodes, prob, seed):
    random.seed(seed)
    edge_list = []
    
    # Generate random directed edges
    for u in range(num_nodes):
        for v in range(num_nodes):
            # Add 'u != v' to prevent self-loops from entering the edge list
            if u != v and random.random() < prob: 
                edge_list.append((u, v))
                
    print("==================================================")
    print(f" GENERATED C++ TEST CODE (Nodes: {num_nodes}, Seed: {seed}), Prob: {prob}")
    print("==================================================")
    print("// Copy and paste this into your C++ test case:")
    for u, v in edge_list:
        print(f"dg.insert_edge({u}, {v});")
        
    # Build and run the simulation
    g = CSRGraph(num_nodes, edge_list)
    simulate_dfs_mem_distance(g, start_id=0)


# ==========================================
# Run the Random Generator
# parameters: (num_nodes, probability_of_edge, random_seed)
# ==========================================

# Generates a tiny 6-node graph with roughly 30% connection probability
generate_and_test_random_graph(num_nodes=30, prob=0.15, seed=42)