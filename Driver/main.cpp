import std;
import Helper;
import Graph;

auto test_get_graph = []{
Small_Graph::csr_graph::csr_weighted_graph g;

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

auto test_dfs_bfs(Small_Graph::csr_weighted_graph& g){
	using namespace Small_Graph;
	struct visitor : default_non_cost_visitor{
		int count{};
		auto find_node(const node n){
			count++;
			std::print("\nfind node {}", n);
			return function_flow::iteration_continue;
		}
		auto find_edge(const node, const edge e){
			count++;
			std::print("\n\tfind edge {}", e);
			return function_flow::iteration_continue;
		}
	};

	visitor v;
	g.dfs_loop(0, v);
	std::print("\n\ndfs count == {}\n\n", v.count);

	v.count = 0;
	g.bfs_loop(0, v);
	std::print("\n\nbfs count == {}", v.count);
}

void test_graph(){
	auto g = test_get_graph();
	test_dfs_bfs(g);
// 	test_ucs(g);
}

int main(void){
	Debug::clear_log();

	test_graph();

	return 0;
}