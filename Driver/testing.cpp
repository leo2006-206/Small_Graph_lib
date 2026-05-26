import std;
import Graph;
import Helper;

#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <unistd.h>

auto load_graph_from_file(const std::string& filename) {
    Small_Graph::csr_graph::csr_weighted_graph g;
    std::ifstream file(filename);
    std::string line;

    // Track which node we are currently building in the CSR
    std::uint32_t curr_node = 0;
	std::uint32_t prev_node = 0xffffffff;

    while (std::getline(file, line)) {
        // 1. Skip comments
        if (line.empty() || line[0] == '%') continue;

        // 2. Convert commas to spaces so stringstream can parse it easily
        std::replace(line.begin(), line.end(), ',', ' ');

        std::uint32_t src, dest;
        double raw_weight;
        std::istringstream iss(line);

        if (iss >> src >> dest >> raw_weight) {
            // 3. Fix 1-based indexing (File's Node 1 becomes your Node 0)
            src -= 1;
            dest -= 1;

            // 4. Scale the floating point weight to your uint32_t engine
            std::uint32_t weight = static_cast<std::uint32_t>(raw_weight * 10);

			// std::print("\ncurr= {}, prev= {}, src= {}, dest= {}, wei= {}, raw_wei= {}",
			// 	curr_node,prev_node, src, dest, weight, raw_weight);

			while(curr_node != src && curr_node < (src - 1)){
				curr_node = g.add_node_without_edge();
				// std::print("\n\tadd empty node");
			}

			if(prev_node == curr_node){
				g.add_last_node_edge({dest, weight});
				// std::print("\n\tadd edge to last node");
			}
			else{
				g.add_node_with_edge({dest, weight});
				// std::print("\n\tadd edge to new node");
			}

			prev_node = curr_node;
        }
    }

	return g;
}

auto ER_random_graph(std::size_t num_node, float prob){
	if(prob < 0.0 || prob > 1.0)
		return Small_Graph::csr_weighted_graph{};

	// std::mt19937 mt{std::random_device{}()};
	std::mt19937 mt{123};
	Small_Graph::csr_weighted_graph g;

	std::uniform_int_distribution<std::uint32_t> wei_dis{0, 1 << 12};
	std::uniform_real_distribution<float> prob_dis{0.0, 1.0};

	for(std::size_t i = 0; i < num_node; ++i){
		bool inserted = false;
		for(std::size_t j = 0; j < num_node; ++j){
			if(i == j)	continue;
			if(prob_dis(mt) < prob){
				if(!inserted){
					g.add_node_with_edge({std::uint32_t(j), wei_dis(mt)});
					inserted = true;
				}
				else{
					g.add_last_node_edge({std::uint32_t(j), wei_dis(mt)});
				}
			}
			
		}
		if(inserted == false){
			g.add_node_without_edge();
		}
	}

	return g;
}

void print_graph_stat(Small_Graph::csr_weighted_graph& g){
	 // Optional: Print to verify the size matches the file's metadata!
    std::print("\nLoaded {} nodes!\n", g._node_vec.size());
	std::print("Loaded edges num = {}\n", g._edge_vec.size());

	std::size_t num_edge{};
	for(auto e : g.edges_range()){
		if(e._dest_id != Small_Graph::NULL_EDGE._dest_id){
			num_edge++;
		}
	}
	std::print("Loaded real edges num = {}\n", num_edge);
}

auto find_best_source_dest(Small_Graph::csr_weighted_graph& g){

	struct BestTargetVisitor : Small_Graph::default_non_cost_visitor{
		mutable std::uint32_t best_target = 0;
		auto find_node([[maybe_unused]] const std::uint32_t n) const {
			best_target = n;
			return Small_Graph::function_flow::iteration_continue;
		}
	};

	std::uint32_t best_start = 0;
    std::size_t max_edges = 0;
    for(std::uint32_t i = 0; i < g._node_vec.size(); ++i){
        auto edges = g.node_edges_range(i);
        if (edges.size() > max_edges) {
            max_edges = edges.size();
            best_start = i;
        }
    }

    BestTargetVisitor b;
	b.best_target = best_start;
    g.bfs_loop(best_start, b);

    std::uint32_t best_target = b.best_target;
    std::print("\nBest start node: {} with {} edges. Target node: {} (furthest reachable)\n", best_start, max_edges, best_target);

	return std::pair{best_start, best_target};
}

void stat_v(Small_Graph::csr_weighted_graph& g){
	struct StatVisitor : Small_Graph::default_non_cost_visitor {
		std::size_t node_count{0};
		std::size_t edge_count{0};

		auto find_node([[maybe_unused]] const std::uint32_t n){
			node_count++;
			return Small_Graph::function_flow::iteration_continue;
		}
		auto find_edge([[maybe_unused]] const std::uint32_t n, [[maybe_unused]] const Small_Graph::edge& e){
			edge_count++;
			return Small_Graph::function_flow::iteration_continue;
		}
	};

	struct CostStatVisitor : Small_Graph::default_cost_visitor<std::uint32_t> {
		mutable std::size_t node_count{0};
		mutable std::size_t edge_count{0};
		mutable std::uint32_t total_cost{0};

		auto find_node([[maybe_unused]] const std::uint32_t n, [[maybe_unused]] const std::uint32_t w) const {
			node_count++;
			return Small_Graph::function_flow::iteration_continue;
		}
		auto find_edge([[maybe_unused]] const std::uint32_t n, [[maybe_unused]] const std::uint32_t w, [[maybe_unused]] const Small_Graph::edge& e) const {
			edge_count++;
			return Small_Graph::function_flow::iteration_continue;
		}
		auto find_edge([[maybe_unused]] const std::uint32_t n, [[maybe_unused]] const Small_Graph::edge& e, [[maybe_unused]] const std::uint32_t w) const {
			edge_count++;
			return Small_Graph::function_flow::iteration_continue;
		}
		auto cal_cost([[maybe_unused]] const std::uint32_t n, [[maybe_unused]] const std::uint32_t w, [[maybe_unused]] const Small_Graph::edge& e) const {
			total_cost += e._weight; 
			return w + e._weight;
		}
	};

	// {
	// 	std::print("\n=== DFS / BFS Tests ===\n");
	// 	Timing::print_timer _;
	// 	StatVisitor v;
	// 	auto s = g._node_vec.size() >> 2;
	// 	for(std::size_t i = 0; i < s; ++i){
	// 		v.node_count = 0; v.edge_count = 0;
	// 		g.dfs_loop(i, v);
	// 	}
	// 	std::print("DFS Stat: Node count: {}, Edge count: {}\n", v.node_count, v.edge_count);
	// }

	// {
	// 	Timing::print_timer _;
	// 	StatVisitor v;
	// 	auto s = g._node_vec.size() >> 2;
	// 	for(std::size_t i = 0; i < s; ++i){
	// 		v.node_count = 0; v.edge_count = 0;
	// 		g.bfs_loop(i, v);
	// 	}
	// 	std::print("BFS Stat: Node count: {}, Edge count: {}\n", v.node_count, v.edge_count);
	// }

	{
		std::print("\n=== UCS Tests ===\n");
		Timing::print_timer _;
		CostStatVisitor v;
		auto s = g._node_vec.size() >> 2;
		for(std::size_t i = 0; i < s; ++i){
			v.node_count = 0; v.edge_count = 0; v.total_cost = 0;
			g.ucs_loop(i, v);
		}
		std::print("UCS Stat: Node count: {}, Edge count: {}, Total Cost Evaluated: {}\n", 
            v.node_count, v.edge_count, v.total_cost);
	}
}

void search_v(Small_Graph::csr_weighted_graph& g){
	struct SearchVisitor : Small_Graph::default_non_cost_visitor {
		std::uint32_t TargetDest = 0;
		mutable std::size_t step_count = 0;
		mutable bool found = false;

		auto find_node([[maybe_unused]] const std::uint32_t n) const {
			step_count++;
			if (n == TargetDest) {
				found = true;
				return Small_Graph::function_flow::function_return;
			}
			return Small_Graph::function_flow::iteration_continue;
		}
		auto find_edge([[maybe_unused]] const std::uint32_t n, [[maybe_unused]] const Small_Graph::edge& e) const {
			step_count++;
			if (e._dest_id == TargetDest) {
				found = true;
				return Small_Graph::function_flow::function_return;
			}
			return Small_Graph::function_flow::iteration_continue;
		}
	};

	struct CostSearchVisitor : Small_Graph::default_cost_visitor<std::uint32_t> {
		std::uint32_t TargetDest = 0;
		mutable std::size_t step_count = 0;
		mutable std::uint32_t reached_cost = 0;
		mutable bool found = false;

		auto find_node([[maybe_unused]] const std::uint32_t n, [[maybe_unused]] const std::uint32_t w) const {
			step_count++;
			if (n == TargetDest) {
				found = true;
				reached_cost = w;
				return Small_Graph::function_flow::function_return;
			}
			return Small_Graph::function_flow::iteration_continue;
		}
		auto find_edge([[maybe_unused]] const std::uint32_t n, [[maybe_unused]] const std::uint32_t w, [[maybe_unused]] const Small_Graph::edge& e) const {
			step_count++;
			if (e._dest_id == TargetDest) {
				found = true;
				reached_cost = w + e._weight;
				return Small_Graph::function_flow::function_return;
			}
			return Small_Graph::function_flow::iteration_continue;
		}
		auto find_edge([[maybe_unused]] const std::uint32_t n, [[maybe_unused]] const Small_Graph::edge& e, [[maybe_unused]] const std::uint32_t w) const {
			step_count++;
			if (e._dest_id == TargetDest) {
				found = true;
				reached_cost = w + e._weight;
				return Small_Graph::function_flow::function_return;
			}
			return Small_Graph::function_flow::iteration_continue;
		}
	};

	{
		std::print("\n--- DFS Search Test ---");
		Timing::print_timer _;
		SearchVisitor v;
		v.TargetDest = 2086;
		for(int i = 0; i < 10000; ++i){
			v.step_count = 0; v.found = false;
			g.dfs_loop(4872, v);
		}
		std::print("DFS Search(2086): Found: {}, Steps: {}\n", v.found, v.step_count);
	}

	{
		std::print("\n--- BFS Search Test ---");
		Timing::print_timer _;
		SearchVisitor v;
		v.TargetDest = 2086;
		for(int i = 0; i < 10000; ++i){
			v.step_count = 0; v.found = false;
			g.bfs_loop(4872, v);
		}
		std::print("BFS Search(2086): Found: {}, Steps: {}\n", v.found, v.step_count);
	}

	{
		std::print("\n--- UCS Search Test ---");
		Timing::print_timer _;
		CostSearchVisitor v;
		v.TargetDest = 2086;
		for(int i = 0; i < 1000; ++i){
			v.step_count = 0; v.found = false; v.reached_cost = 0;
			g.ucs_loop(4872, v);
		}
		std::print("UCS Search(2086): Found: {}, Steps: {}, Reached Cost: {}\n", 
        	v.found, v.step_count, v.reached_cost);
	}
}

int main(void){
	Timing::perf_control pc;

	// auto g = load_graph_from_file("./Dataset/advogato/advogato.edges");
	auto g = ER_random_graph(10000, 0.2f);
	print_graph_stat(g);

	pc.start();

	stat_v(g);
	// search_v(g);

	pc.finish();

	return 0;
}
