module;

export module Graph:dynamic_graph;

import std;
import :function;

export namespace Small_Graph::dynamic_graph{
	struct dyn_node{
		using node_id_t = std::uint64_t;
		node_id_t	_id;
	};

	struct dyn_edge{
		using node_id_t = std::uint64_t;
		node_id_t	_source;
		node_id_t	_dest;
	};

	struct dyn_w_edge{
		using node_id_t = std::uint64_t;
		using weight_t	= std::uint64_t;
		node_id_t	_source;
		node_id_t	_dest;
		
	};
}