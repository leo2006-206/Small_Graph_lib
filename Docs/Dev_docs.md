# C++ Graph Library API Documentation
**Date**: 24 May 2026 (Updated)

## Overview
The `Graph` module (`Module/Graph`) provides a Compressed Sparse Row (CSR) based weighted graph implementation and several generic traversal algorithms (DFS, BFS, UCS). The graph nodes are represented by `std::uint32_t` IDs.

## Data Types

### `Graph::csr_graph::node`
Alias for `std::uint32_t`, represents the node ID.

### `Graph::csr_graph::edge`
Struct representing a directed weighted edge.
- **Types**:
  - `cost_type`: Alias for `std::uint32_t` representing the cost definition.
- **Fields**:
  - `_dest_id` (`node`): Destination node ID.
  - `_weight` (`cost_type`): Edge weight.

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

### `dfs_loop` (Depth-First Search) & `bfs_loop` (Breadth-First Search)
Traverses the graph using Depth-First or Breadth-First Search.
- **Template Args**: 
  - `non_cost_visitor`: A struct satisfying `is_non_cost_graph_visitor` concept (default `default_non_cost_visitor`).
- **Args**:
  - `start_id`: Node ID to begin the search.
  - `graph_visitor`: A visitor object containing `find_node(node)`, `find_edge(node, edge)`, and `end_node(node)` methods that return `function_flow`.

### `ucs_loop` (Uniform Cost Search)
Traverses the graph using Uniform Cost Search (like Dijkstra's algorithm).
- **Template Args**:
  - `cost_visitor`: A struct satisfying `is_cost_graph_visitor` concept (default `default_cost_visitor`).
  - Must define `cost_type` inside the visitor.
- **Args**:
  - `start_id`: Node ID to begin UCS.
  - `graph_visitor`: A visitor object containing `find_node(node, cost)`, `find_edge(node, edge, cost)`, `cal_cost(node, cost, edge)`, and `end_node(node, cost)` methods.

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

    struct BfsVisitor : Graph::function::default_non_cost_visitor {
        auto find_node(const Graph::csr_graph::node n) const {
            std::println("BFS Discovered Node: {}", n);
            return gf::iteration_continue;
        }
    };

    // Perform BFS traversal starting from Node 0
    graph.bfs_loop(0, BfsVisitor{});
    
    return 0;
}
```
