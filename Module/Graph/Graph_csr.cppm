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
	node			_dest_id;
	std::uint32_t	_weight;

	constexpr edge(
		std::uint32_t id,
		std::uint32_t weight
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

	template<	
		typename discover_node	= gf::unused_node_func,
		typename examine_edge	= gf::unused_edge_func,
		typename finish_node	= gf::unused_node_func
	>
	requires
		gf::node_function<discover_node, node> &&
		gf::edge_function<examine_edge, node, edge> &&
		gf::node_function<finish_node, node>
	void dfs_loop(
		node 			start_id,
		discover_node	find_node	={},
		examine_edge	find_edge	={},
		finish_node		end_node	={}
	){
		if(!node_contains(start_id)){
			return;
		}

		std::vector<node> stack;
		std::vector<std::bitset<64>> visit_vec;
		visit_vec.resize((_node_vec.size() >> 6) + 1);
		stack.reserve(_node_vec.size() >> 1);

		stack.emplace_back(start_id);
		visit_vec[start_id >> 6].set(start_id & (64 - 1), true);

		while(!stack.empty()){
			const auto current_node = stack.back();
			stack.pop_back();

			auto flow_1 = find_node(current_node);
			GRAPH_HANDLE_FLOW(flow_1)	//macro

			for(const auto& current_edge : node_edges_range(current_node)){
				
				if(!node_contains(current_edge._dest_id))	continue;

				auto flow_2 = find_edge(current_node, current_edge);
				GRAPH_HANDLE_FLOW(flow_2)	//macro

				auto dest = current_edge._dest_id;

				if(visit_vec[dest >> 6].test(dest & (64 - 1)) == false && node_contains(dest)){
					visit_vec[dest >> 6].set(dest & (64 - 1), true);
					stack.emplace_back(dest);
				}
			}

			auto flow_3 = end_node(current_node);
			GRAPH_HANDLE_FLOW(flow_3)	//macro
		}
	}

	template<	
		typename discover_node	= gf::unused_node_func,
		typename examine_edge	= gf::unused_edge_func,
		typename finish_node	= gf::unused_node_func
	>
	requires
		gf::node_function<discover_node, node> &&
		gf::edge_function<examine_edge, node, edge> &&
		gf::node_function<finish_node, node>
	void bfs_loop(
		node 			start_id,
		discover_node	find_node	={},
		examine_edge	find_edge	={},
		finish_node		end_node	={}
	){
		if(!node_contains(start_id)){
			return;
		}
		
		std::queue<node, std::deque<node>> queue;
		std::vector<std::bitset<64>> visit_vec;
		visit_vec.resize((_node_vec.size() >> 6) + 1);

		queue.emplace(start_id);
		visit_vec[start_id >> 8].set(start_id & (64 - 1), true);

		while(!queue.empty()){
			const auto current_node = queue.front();
			queue.pop();

			auto flow_1 = find_node(current_node);
			GRAPH_HANDLE_FLOW(flow_1)	//macro

			for(const auto& current_edge : node_edges_range(current_node)){
				
				if(!node_contains(current_edge._dest_id))	continue;

				auto flow_2 = find_edge(current_node, current_edge);
				GRAPH_HANDLE_FLOW(flow_2)	//macro

				auto dest = current_edge._dest_id;

				if(visit_vec[dest >> 6].test(dest & (64 - 1)) == false){
					visit_vec[dest >> 6].set(dest & (64 - 1), true);
						queue.emplace(dest);
				}
				
			}

			auto flow_3 = end_node(current_node);
			GRAPH_HANDLE_FLOW(flow_3)	//macro
		}
	}

	template<
		typename discover_node	= gf::unused_node_cost_func,
		typename examine_edge	= gf::unused_edge_cost_func,
		typename compute_cost	= gf::default_cost_func,
		typename finish_node	= gf::unused_node_cost_func
	>
	requires
		gf::node_cost_function<discover_node, node, std::invoke_result_t<compute_cost, const node, const edge>> &&
		gf::edge_cost_function<examine_edge, node, edge, std::invoke_result_t<compute_cost, const node, const edge>> &&
		gf::cost_function<compute_cost, node, edge> &&
		gf::node_cost_function<finish_node, node, std::invoke_result_t<compute_cost, const node, const edge>>
	void ucs_loop(
		node 			start_id,
		discover_node	find_node	={},
		examine_edge	find_edge	={},
		compute_cost	cal_cost	={},
		finish_node		end_node	={}
	){
		if(!node_contains(start_id)){
			return;
		}
		using cost_t = std::invoke_result_t<compute_cost, const node, const edge>;
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

			auto flow_1 = find_node(current_node, current_cost);
			GRAPH_HANDLE_FLOW(flow_1)	//macro

			for(const auto& current_edge : node_edges_range(current_node)){
				
				if(!node_contains(current_edge._dest_id))	continue;

				auto flow_2 = find_edge(current_node, current_edge, current_cost);
				GRAPH_HANDLE_FLOW(flow_2)	//macro

				cost_t next_cost = cal_cost(current_node, current_edge) + current_cost;

				if (next_cost < cost_vec[current_edge._dest_id]) {
                    cost_vec[current_edge._dest_id] = next_cost;
					heap.emplace(next_cost, current_edge._dest_id);
                }
			}

			auto flow_3 = end_node(current_node, current_cost);
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
		for(auto& prev_edge : node_edges_range(current_node)){
			if(prev_edge._dest_id == new_edge._dest_id && prev_edge._weight > new_edge._weight){
				prev_edge._weight = new_edge._weight;
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