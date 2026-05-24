module;

#include <cstdint>
#include <cassert>

export module Graph:csr_graph;

import std;
import :function;

namespace gf = Graph::function;

#define GRAPH_HANDLE_FLOW(flow_state) \
    if((flow_state) == gf::function_flow::function_return)	return;\
    else if((flow_state) == gf::function_flow::iteration_break)	break;\
    else if((flow_state) == gf::function_flow::iteration_skip)	continue;\
	//else if(flow == gf::function_flow::iteration_continue)

export namespace Graph::csr_graph{

using node = std::uint32_t;

struct edge{
	using cost_type = std::uint32_t;
	node		_dest_id;
	cost_type	_weight;

	constexpr edge(
		node		id,
		cost_type	weight
	): _dest_id(id), _weight(weight){}
};


//used for node without edge
constexpr edge NULL_EDGE{
	std::numeric_limits<std::uint32_t>::max(),
	std::numeric_limits<std::uint32_t>::max()
};

struct csr_weighted_graph{

	// _node_vec[ node_id ] = starting index of edges in _edge_vec
	std::vector<node>	_node_vec;
	std::vector<edge>	_edge_vec;

	csr_weighted_graph();

	bool			node_contains(node source);
	std::span<node>	nodes_range();

	bool			edge_contains(node source, edge target);
	std::span<edge> edges_range();

	std::span<edge> node_edges_range(node node_index);

	node			add_node_without_edge();
	node			add_node_with_edge(edge new_edge);
	node			add_last_node_edge(edge new_edge);

	template<typename non_cost_visitor = function::default_non_cost_visitor>
	requires function::is_non_cost_graph_visitor<non_cost_visitor, node, edge>
	void dfs_loop(node start_id, const non_cost_visitor graph_visitor = {}){
		if(!node_contains(start_id)){
			return;
		}

		constexpr auto cal_offset = [](node id) -> std::uint64_t{
			return 1 << (id & 63);
		};

		std::vector<node> stack;
		std::vector<std::uint64_t> visit_vec;
		visit_vec.resize((_node_vec.size() >> 6) + 1, 0);
		stack.reserve(_node_vec.size() >> 1);

		stack.emplace_back(start_id);
		visit_vec[start_id >> 6] |= cal_offset(start_id);

		while(!stack.empty()){
			const auto current_node = stack.back();
			stack.pop_back();

			auto flow_1 = graph_visitor.find_node(current_node);
			GRAPH_HANDLE_FLOW(flow_1)	//macro

			for(const auto current_edge : node_edges_range(current_node)){
				
				if(current_edge._dest_id == NULL_EDGE._dest_id)	continue;

				auto flow_2 = graph_visitor.find_edge(current_node, current_edge);
				GRAPH_HANDLE_FLOW(flow_2)	//macro

				auto dest = current_edge._dest_id;
				auto& bit_pos = visit_vec[dest >> 6];

				if((bit_pos & cal_offset(dest)) == false && current_edge._dest_id != NULL_EDGE._dest_id){
					bit_pos |= cal_offset(dest);
					stack.emplace_back(dest);
				}
			}

			auto flow_3 = graph_visitor.end_node(current_node);
			GRAPH_HANDLE_FLOW(flow_3)	//macro
		}
	}

	template<typename non_cost_visitor = function::default_non_cost_visitor>
	requires function::is_non_cost_graph_visitor<non_cost_visitor, node, edge>
	void bfs_loop(node start_id, const non_cost_visitor graph_visitor = {}){
		if(!node_contains(start_id)){
			return;
		}

		constexpr auto cal_offset = [](node id) -> std::uint64_t{
			return 1 << (id & 63);
		};
		
		std::queue<node, std::deque<node>> queue;
		std::vector<std::uint64_t> visit_vec;
		visit_vec.resize((_node_vec.size() >> 6) + 1, 0);

		queue.emplace(start_id);
		visit_vec[start_id >> 6] |= cal_offset(start_id);

		while(!queue.empty()){
			const auto current_node = queue.front();
			queue.pop();

			auto flow_1 = graph_visitor.find_node(current_node);
			GRAPH_HANDLE_FLOW(flow_1)	//macro

			for(const auto current_edge : node_edges_range(current_node)){
				
				if(current_edge._dest_id == NULL_EDGE._dest_id)	continue;

				auto flow_2 = graph_visitor.find_edge(current_node, current_edge);
				GRAPH_HANDLE_FLOW(flow_2)	//macro

				auto dest = current_edge._dest_id;
				auto& bit_pos = visit_vec[dest >> 6];

				if((bit_pos & cal_offset(dest)) == false && current_edge._dest_id != NULL_EDGE._dest_id){
					bit_pos |= cal_offset(dest);
					queue.emplace(dest);
				}
				
			}

			auto flow_3 = graph_visitor.end_node(current_node);
			GRAPH_HANDLE_FLOW(flow_3)	//macro
		}
	}

	template<typename cost_visitor = function::default_cost_visitor<edge::cost_type>>
	requires function::is_cost_graph_visitor<cost_visitor, node, edge>
	void ucs_loop(node start_id, const cost_visitor graph_visitor = {}){
		if(!node_contains(start_id)){
			return;
		}
		using cost_t = typename cost_visitor::cost_type;
		using cost_pair = std::pair<cost_t, node>;
		using heap_t = std::priority_queue<cost_pair, std::vector<cost_pair>, std::greater<cost_pair>>;

		constexpr auto lowest = std::numeric_limits<cost_t>::lowest();

		std::vector<cost_t> cost_vec;
		std::vector<cost_pair> vec_container;

		cost_vec.resize(_node_vec.size(), std::numeric_limits<cost_t>::max());
		vec_container.reserve(_node_vec.size() >> 1);
		heap_t heap{std::greater<cost_pair>(), std::move(vec_container)};

		cost_vec[start_id] = lowest;
		heap.emplace(lowest, start_id);

		while(!heap.empty()){
			const auto [current_cost, current_node] = heap.top();
			heap.pop();

			if (current_cost > cost_vec[current_node]) {
                continue; 
            }

			auto flow_1 = graph_visitor.find_node(current_node, current_cost);
			GRAPH_HANDLE_FLOW(flow_1)	//macro

			for(const auto& current_edge : node_edges_range(current_node)){
				
				if(!node_contains(current_edge._dest_id))	continue;

				auto flow_2 = graph_visitor.find_edge(current_node, current_edge, current_cost);
				GRAPH_HANDLE_FLOW(flow_2)	//macro

				cost_t next_cost = graph_visitor.cal_cost(current_node, current_cost, current_edge);

				if (next_cost < cost_vec[current_edge._dest_id]) {
                    cost_vec[current_edge._dest_id] = next_cost;
					heap.emplace(next_cost, current_edge._dest_id);
                }
			}

			auto flow_3 = graph_visitor.end_node(current_node, current_cost);
			GRAPH_HANDLE_FLOW(flow_3)	//macro
		}
	}
};

}

namespace Graph::csr_graph{

	csr_weighted_graph::csr_weighted_graph(){
		_node_vec.reserve(1ll << 8);
		_edge_vec.reserve(1ll << 10);

		// _edge_vec.emplace_back(NULL_EDGE);
	}

	bool csr_weighted_graph::node_contains(node source){
		auto node_size = _node_vec.size();
		if(source < node_size){
			return true;
		}
		else{
			return false;
		}
	}

	std::span<node> csr_weighted_graph::nodes_range(){
		return {_node_vec};
	}

	std::span<edge> csr_weighted_graph::node_edges_range(node node_index){
		// Error checking = not out of bound
		// Return a span of edges for a node
		auto node_size = _node_vec.size();
		if(node_index >= node_size){
			return {};
		}
		if(node_index == node_size - 1){
			auto count = _edge_vec.size() - _node_vec[node_index];
			return std::span<edge>{_edge_vec}.last(count);
		}
		else{
			const node* index_ptr = _node_vec.data() + node_index;
			auto count = *(index_ptr + 1) - *index_ptr;
			return std::span<edge>{_edge_vec}.subspan(*index_ptr, count);
		}
	}

	bool csr_weighted_graph::edge_contains(node source, edge target){
		for(const auto& e : node_edges_range(source)){
			if(e._dest_id == target._dest_id && e._weight == target._weight){
				return true;
			}
		}

		return false;
	}

	std::span<edge> csr_weighted_graph::edges_range(){
		return {_edge_vec};
	}

	// interface start here

	node csr_weighted_graph::add_node_without_edge(){
		return add_node_with_edge(NULL_EDGE);
	}

	node csr_weighted_graph::add_node_with_edge(edge new_edge){
		// Add new node to graph, id == _node_vec.size()
		// Each node require least one edge
		auto new_node = static_cast<node>(_node_vec.size());
		_node_vec.emplace_back( static_cast<node>(_edge_vec.size()) );
		_edge_vec.emplace_back(new_edge);
		return new_node;
	}

	node csr_weighted_graph::add_last_node_edge(edge new_edge){
		// Add edge to the last node
		// If duplicated edge, keep the lower weight one
		
		auto current_node = static_cast<node>(_node_vec.size()) - 1;
		for(auto prev_edge : node_edges_range(current_node)){
			if(prev_edge._dest_id == new_edge._dest_id){
				if(prev_edge._weight > new_edge._weight){
					prev_edge._weight = new_edge._weight;
				}
				return current_node;
			}
		}

		_edge_vec.emplace_back(new_edge);
		return current_node;
	}

}

template <>
struct std::formatter<Graph::csr_graph::edge>{
	constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }
	auto format(const Graph::csr_graph::edge& e, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "(dest: {}, weight: {})", e._dest_id, e._weight);
    }
};

#undef GRAPH_HANDLE_FLOW