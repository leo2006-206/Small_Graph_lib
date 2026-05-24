# Dev Daily Notes

## Format Details
This document serves as a tracking log for the development progress of the Graph module.
- Each code file is represented as a level 2 heading (`##`).
- Code objects (structs, functions, concepts, enum classes, namespaces) are listed under their corresponding files.
- Each code object contains a Markdown table tracking its update history.
- A new update is logged as a row with the date and a brief description.

## Module/Graph/Graph_csr.cppm
|**struct** `edge`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|
|24 May 2026| Added `cost_type` definition and decoupled `_weight` into cost_type.|

|**struct** `csr_weighted_graph`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**function** `csr_weighted_graph::node_contains`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**function** `csr_weighted_graph::nodes_range`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**function** `csr_weighted_graph::edge_contains`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**function** `csr_weighted_graph::edges_range`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**function** `csr_weighted_graph::node_edges_range`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**function** `csr_weighted_graph::add_node_without_edge`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|
|24 May 2026| Fixed add node bug, changed `add_last_node_edge` to `add_node_with_edge(NULL_EDGE)`|

|**function** `csr_weighted_graph::add_node_with_edge`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**function** `csr_weighted_graph::add_last_node_edge`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**function** `csr_weighted_graph::dfs_loop`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|
|24 May 2026| Optimized bitwise loop, removed `node_contains`, and changed to use `is_non_cost_graph_visitor`.|

|**function** `csr_weighted_graph::bfs_loop`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|
|24 May 2026| Optimized bitwise loop, removed `node_contains`, and changed to use `is_non_cost_graph_visitor`.|

|**function** `csr_weighted_graph::ucs_loop`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|
|24 May 2026| Updated method callbacks to accurately pass `current_cost` weight to `examine_edge` and `finish_node`.|
|24 May 2026| Refactored loop to use `is_cost_graph_visitor` visitor pattern instead of three separate lambdas.|

|**struct** `std::formatter<Graph::csr_graph::edge>`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

## Module/Graph/Graph_function.cppm

|**concept** `is_weighted_edge`||
|:-|:-:|
|Date| Description|
|24 May 2026| init|

|**concept** `is_non_cost_graph_visitor`||
|:-|:-:|
|Date| Description|
|24 May 2026| init|

|**concept** `is_cost_graph_visitor`||
|:-|:-:|
|Date| Description|
|24 May 2026| init|

|**struct** `default_non_cost_visitor`||
|:-|:-:|
|Date| Description|
|24 May 2026| init|

|**struct** `default_cost_visitor`||
|:-|:-:|
|Date| Description|
|24 May 2026| init|

|**enum class** `function_flow`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**concept** `is_node`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**concept** `is_weight`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**concept** `is_edge`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**concept** `node_function`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**concept** `edge_function`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**concept** `node_cost_function`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**concept** `cost_function`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**concept** `edge_cost_function`||
|:-|:-:|
|Date| Description|
|24 May 2026| init|

|**concept** `non_cost_graph_visitor`||
|:-|:-:|
|Date| Description|
|24 May 2026| init|

|**concept** `cost_graph_visitor`||
|:-|:-:|
|Date| Description|
|24 May 2026| init|

|**struct** `unused_node_func`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**struct** `unused_edge_func`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**struct** `unused_node_cost_func`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

|**struct** `unused_edge_cost_func`||
|:-|:-:|
|Date| Description|
|24 May 2026| init|

|**struct** `default_cost_func`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|

## Module/Graph/Graph.cppm
|*Module umbrella file, re-exports components only* == *namespace* `Small_Graph`||
|:-|:-:|
|Date| Description|
|23 May 2026| init|
|24 May 2026| Added alias namespace combining `Graph::csr_graph` and `Graph`.|

