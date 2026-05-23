import std;
import Helper;
import Graph;

auto test_get_graph = []{
	Graph::csr_graph::csr_weighted_graph g;

	// Node 0
	g.add_node_with_edge({1, 4});
	g.add_last_node_edge({2, 2});

	// Node 1
	g.add_node_with_edge({3, 5});

	// Node 2
	g.add_node_with_edge({1, 10});
	g.add_last_node_edge({1, 1});
	g.add_last_node_edge({3, 8});
	g.add_last_node_edge({4, 10});

	// Node 3
	g.add_node_with_edge({4, 2});

	// Node 4
	g.add_node_without_edge();

	std::print("\nPrint nodes:\n\t{}\n\n", g.nodes_range());

	for(auto& e : g.edges_range()){
		std::print("{}\n", e);
	}

	return g;
};

auto test_graph = []{
	using namespace Graph::csr_graph;
	using namespace Graph::function;
	auto g = test_get_graph();

	std::bitset<64> bs;

	auto first = [&](const node id){
		Debug::debug_print_log("pop node {} from container",id);
		// if(bs[id] == 0){
		// 	bs[id] = 1;
		// 	return function_flow::iteration_continue;
		// }
		return function_flow::iteration_continue;
	};

	constexpr auto second = [](const node v, const edge e){
		Debug::debug_print_log("\tfind edge fron node {} to node {} with cost {}",v, e._dest_id, e._weight);
		return function_flow::iteration_continue;
	};

	std::print("\n\ndfs start\n");

	g.dfs_loop(0, first, second);

	std::print("\n\nbfs start\n");

	bs.reset();
	g.bfs_loop(0, first, second);

	auto new_first = [&](const node id, const std::uint32_t cost){
		Debug::debug_print_log("pop node {} with total cost {} from container",id, cost);
		if(id == 4){
			return function_flow::function_return;
		}
		return function_flow::iteration_continue;
	};


	std::print("\n\nucs start\n");

	bs.reset();
	g.ucs_loop(0, new_first, second);
};

int main(void){
	Debug::clear_log();

	test_graph();

	return 0;
}