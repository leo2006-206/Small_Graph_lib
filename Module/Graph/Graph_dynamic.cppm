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
};

struct dyn_dir_graph{
	std::vector<dyn_node>		_node_vec;
	std::size_t					_num_edge;

	bool						node_contains(node_id_t id) const;
	bool						edge_contains(node_id_t source, node_id_t dest) const;

	std::span<dyn_node>			node_range();
	std::span<const dyn_node>	node_range() const;
	std::span<node_id_t>		node_edges_range(node_id_t id);
	std::span<const node_id_t>	node_edges_range(node_id_t id) const;

	dyn_node*					access_node_ptr(node_id_t id);
	const dyn_node*				access_node_ptr(node_id_t id) const;

	dyn_node*					insert_node_ord(node_id_t id);
	dyn_node*					insert_node_unord(node_id_t id);
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


// dyn_dir_graph start here

auto dyn_node_cmp = [](const dyn_node& a, const dyn_node& b) -> bool{
	// std::ranges::lower_bound(_node_vec, source, dyn_node_cmp, &dyn_node::_id)
	return (a._id < b._id);
};

bool dyn_dir_graph::node_contains(node_id_t id) const{
	const auto node_ptr = access_node_ptr(id);
	return (node_ptr != nullptr);;
}

bool dyn_dir_graph::edge_contains(node_id_t source, node_id_t dest) const{
	const auto node_ptr = access_node_ptr(source);
	if(node_ptr == nullptr){
		return false;
	}

	return node_ptr->edge_contains(dest);
}

std::span<dyn_node> dyn_dir_graph::node_range(){
	return (_node_vec);
}

std::span<const dyn_node> dyn_dir_graph::node_range() const{
	return (_node_vec);
}

std::span<node_id_t> dyn_dir_graph::node_edges_range(node_id_t id){
	auto node_ptr = access_node_ptr(id);
	if(node_ptr == nullptr){
		return {};
	}
	return node_ptr->edges_range();
}

std::span<const node_id_t> dyn_dir_graph::node_edges_range(node_id_t id) const{
	const auto node_ptr = access_node_ptr(id);
	if(node_ptr == nullptr){
		return {};
	}
	return node_ptr->edges_range();
}

dyn_node* dyn_dir_graph::access_node_ptr(node_id_t id){
	auto node = std::ranges::lower_bound(_node_vec, id, dyn_node_cmp, &dyn_node::_id);
	if(node == _node_vec.end() || node->_id != id){
		return nullptr;
	}
	return &(*node);
}

const dyn_node*	dyn_dir_graph::access_node_ptr(node_id_t id) const{
	const auto node = std::ranges::lower_bound(_node_vec, id, dyn_node_cmp, &dyn_node::_id);
	if(node == _node_vec.end() || node->_id != id){
		return nullptr;
	}
	return &(*node);
}

}