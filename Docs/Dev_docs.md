# C++ Graph Library API Documentation
**Date**: 23 May 2026

## Overview
The `Graph` module (`Module/Graph`) provides a Compressed Sparse Row (CSR) based weighted graph implementation and several generic traversal algorithms (DFS, BFS, UCS). The graph nodes are represented by `std::uint32_t` IDs.

## Data Types

### `Graph::csr_graph::node`
Alias for `std::uint32_t`, represents the node ID.

### `Graph::csr_graph::edge`
Struct representing a directed weighted edge.
- **Fields**:
  - `_dest_id` (`node`): Destination node ID.
  - `_weight` (`std::uint32_t`): Edge weight.

### `Graph::function::function_flow`
Enum for loop control in the custom lambdas passed to traversal functions.
- `function_return`: Stop execution and return from the traversal function.
- `iteration_continue`: Continue execution.
- `iteration_skip`: Skip current iteration of the loop (equivalent to `continue`).
- `iteration_break`: Break current loop.

---

## Class: `csr_weighted_graph`

### Construction & State
- **`csr_weighted_graph()`**: Constructs an empty graph.

### Queries
- **`bool node_contains(node source)`**: Checks if the graph contains a node with ID `source`.
- **`std::span<node> nodes_range()`**: Returns a span of starting edge offsets for all nodes.
- **`bool edge_contains(node source, edge target)`**: Checks if the graph contains a specific `target` edge from `source` node.
- **`std::span<edge> edges_range()`**: Returns a combined span of all edges across all nodes.
- **`std::span<edge> node_edges_range(node node_index)`**: Returns a contiguous span of outbound edges for the given `node_index`.

### Modifiers
- **`node add_node_without_edge()`**: Adds a new node without any outbound edges. Returns the new node ID.
- **`node add_node_with_edge(edge new_edge)`**: Adds a new node with a single outbound edge. Returns the new node ID.
- **`node add_last_node_edge(edge new_edge)`**: Adds an outbound edge to the last added node. Updates the weight if an edge to the same destination already exists and the new weight is lower. Returns the last node ID.

---

## Graph Traversals

The graph provides generic loop methods for traversal algorithms. You can inject custom functors or lambdas to hook into the discovery, examination, and termination phases.

### `dfs_loop` (Depth-First Search)
Traverses the graph using Depth-First Search.

- **Template Args**: 
  - `discover_node`: `(node) -> function_flow`
  - `examine_edge`: `(node, edge) -> function_flow`
  - `finish_node`: `(node) -> function_flow`
- **Args**:
  - `start_id`: Node ID to begin DFS.
  - `find_node`, `find_edge`, `end_node`: Callback functor instances or lambdas.

### `bfs_loop` (Breadth-First Search)
Traverses the graph using Breadth-First Search.

- **Template Args**:
  - `discover_node`: `(node) -> function_flow`
  - `examine_edge`: `(node, edge) -> function_flow`
  - `finish_node`: `(node) -> function_flow`
- **Args**:
  - `start_id`: Node ID to begin BFS.
  - `find_node`, `find_edge`, `end_node`: Callback instances.

### `ucs_loop` (Uniform Cost Search)
Traverses the graph using Uniform Cost Search (like Dijkstra's algorithm).

- **Template Args**:
  - `discover_node`: `(node, cost_type) -> function_flow`
  - `examine_edge`: `(node, edge) -> function_flow`
  - `compute_cost`: `(node, edge) -> cost_type` (Defaults to grabbing the edge's weight)
  - `finish_node`: `(node) -> function_flow`
- **Args**:
  - `start_id`: Node ID to begin UCS.
  - `find_node`, `find_edge`, `cal_cost`, `end_node`: Callback instances.

---

## Simple Example

```cpp
import std;
import Graph;

using gf = Graph::function::function_flow;
using edge = Graph::csr_graph::edge;

int main() {
    Graph::csr_graph::csr_weighted_graph graph;
    
    // Add Node 0 with an edge to Node 1 (weight 10)
    graph.add_node_with_edge(edge(1, 10));
    // Add another edge to Node 0 towards Node 2 (weight 20)
    graph.add_last_node_edge(edge(2, 20));

    // Add Node 1 with an edge to Node 2 (weight 5)
    graph.add_node_with_edge(edge(2, 5));

    // Add Node 2 without any outbound edges
    graph.add_node_without_edge();

    // Perform BFS traversal starting from Node 0
    auto on_discover = [](const Graph::csr_graph::node n) {
        std::println("BFS Discovered Node: {}", n);
        return gf::iteration_continue;
    };
    
    graph.bfs_loop(
        0, 
        on_discover 
    );
    
    return 0;
}
```
