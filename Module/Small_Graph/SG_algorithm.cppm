module;

export module Small_Graph:algorithm;

import std;
import :core;
import :utility;


export namespace SG {

template<typename graph_t>
concept is_node_edges_range_graph = requires(const graph_t& g,SG::node_id_t id) {
	{ g.node_contains(id) }	-> std::same_as<bool>;
	{ g.node_size() }		-> std::same_as<std::size_t>;
	{ g.edge_size() }		-> std::same_as<std::size_t>;

    { g.node_edges_range(id) }	-> std::ranges::sized_range;
    requires std::same_as<
        std::ranges::range_value_t<decltype(g.node_edges_range(id))>, 
        SG::node_id_t
    >;

	requires std::unsigned_integral<SG::node_id_t>;
};

// __attribute__((noinline))
void dfs_loop(
	const is_node_edges_range_graph	auto&	graph,
	std::invocable<SG::alone_edge>	auto&&	discover_edge,
	SG::node_id_t							start_id
){	
	if(!graph.node_contains(start_id)){
		return;
	}

	constexpr auto cal_offset = [](node_id_t id) -> std::uint64_t{
		return 1 << (id & 63);
	};

	std::vector<node_id_t> stack;
	std::vector<std::uint64_t> visit_vec;
	visit_vec.resize((graph.node_size() >> 6) + 1, 0);
	stack.reserve(graph.node_size() >> 1);

	stack.emplace_back(start_id);
	visit_vec[start_id >> 6] |= cal_offset(start_id);

	while(!stack.empty()){
		const auto current_node = stack.back();
		stack.pop_back();

		for(const auto current_edge : graph.node_edges_range(current_node)){

			discover_edge(alone_edge{current_node, current_edge});

			auto offset = cal_offset(current_edge);
			auto& bit_pos = visit_vec[current_edge >> 6];

			if((bit_pos & offset) == false)[[unlikely]]{
				bit_pos |= offset;
				stack.emplace_back(current_edge);
			}
		}
	}
}

constexpr SG::utility::mem_distance dfs_loop_mem_dis(
	const is_node_edges_range_graph	auto&	graph,
	SG::node_id_t							start_id
){	
	if(!graph.node_contains(start_id)){
		return {};
	}

	constexpr auto cal_offset = [](node_id_t id) -> std::uint64_t{
		return 1 << (id & 63);
	};

	std::vector<node_id_t> stack;
	std::vector<std::uint64_t> visit_vec;
	visit_vec.resize((graph.node_size() >> 6) + 1, 0);
	stack.reserve(graph.node_size() >> 1);

	SG::utility::mem_distance md{};
	stack.emplace_back(start_id);
	visit_vec[start_id >> 6] |= cal_offset(start_id);

	while(!stack.empty()){
		const auto current_node = stack.back();
		stack.pop_back();

		for(const auto& current_edge : graph.node_edges_range(current_node)){

			md(
				alone_edge{current_node, current_edge},
				reinterpret_cast<std::intptr_t>(&current_edge)
			);

			auto offset = cal_offset(current_edge);
			auto& bit_pos = visit_vec[current_edge >> 6];

			if((bit_pos & offset) == false)[[unlikely]]{
				bit_pos |= offset;
				stack.emplace_back(current_edge);
			}
		}
	}

	return md;
}

}