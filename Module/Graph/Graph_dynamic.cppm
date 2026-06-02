module;

export module Graph:dynamic_graph;

import std;
import :function;

export namespace Small_Graph::dynamic_graph{
using node_id_t = std::uint64_t;

constexpr auto get_invalid_node(){
	return std::numeric_limits<node_id_t>::max();
}

struct alone_edge{
	node_id_t	_source;
	node_id_t	_dest;

	alone_edge() = delete;
	constexpr alone_edge(node_id_t source, node_id_t dest)
	: _source(source), _dest(dest){}

	constexpr static alone_edge get_invalid(){
		constexpr auto lim = get_invalid_node();
		return {lim, lim};
	}
};

struct dyn_node{
	using edge_vec_t = std::vector<node_id_t /*edge dest*/>;

	node_id_t	_id;
	edge_vec_t	_edge_vec;

	dyn_node() = delete;
	constexpr dyn_node(node_id_t id) : _id(id){};

	bool						edge_contains(node_id_t dest) const;

	std::span<node_id_t>		edges_range();
	std::span<const node_id_t>	edges_range() const;

	bool						insert_edge(node_id_t dest);
	bool						remove_edge(node_id_t dest);
	void						sort_edge();
};

struct dyn_dir_graph{
	using node_table_t		= std::unordered_set<dyn_node>;
	using node_table_it_t	= node_table_t::iterator;
	using node_table_cit_t	= node_table_t::const_iterator;

	node_table_t				_node_map;

	bool						node_contains(node_id_t id) const;
	bool						edge_contains(node_id_t source, node_id_t dest) const;

	std::size_t					node_size() const;

	std::span<dyn_node>			node_range();
	std::span<const dyn_node>	node_range() const;
	std::span<node_id_t>		node_edges_range(node_id_t id);
	std::span<const node_id_t>	node_edges_range(node_id_t id) const;

	node_table_it_t				access_node_it(node_id_t id);
	node_table_cit_t			access_node_it(node_id_t id) const;

	node_table_it_t				insert_node(node_id_t id);
	bool						remove_node(node_id_t id);

	bool						insert_edge(node_id_t source, node_id_t dest);
	bool						remove_edge(node_id_t source, node_id_t dest);
};

}

namespace Small_Graph::dynamic_graph {

// dyn_node start here

bool dyn_node::edge_contains(node_id_t dest) const{
	for(const auto edge_dest : _edge_vec){
		if(dest == edge_dest){
			return true;
		}
	}
	return false;
}

std::span<node_id_t> dyn_node::edges_range(){
	return (_edge_vec);
}

std::span<const node_id_t> dyn_node::edges_range() const{
	return (_edge_vec);
}

bool dyn_node::insert_edge(node_id_t dest){
	if(dest == _id || edge_contains(dest) == true){
		return false;
	}
	_edge_vec.emplace_back(dest);
	return true;
}

bool dyn_node::remove_edge(node_id_t dest){
	return (std::erase(_edge_vec, dest) > 0);
}

void dyn_node::sort_edge(){
	auto cmp = [](node_id_t a, node_id_t b)-> bool{
		return a > b;
	};

	std::ranges::sort(_edge_vec, cmp);
}

// dyn_dir_graph start here


}