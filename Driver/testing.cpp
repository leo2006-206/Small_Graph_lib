import std;
import Graph;
import Helper;

auto load_graph_from_file(const std::string& filename) {
    Graph::csr_graph::csr_weighted_graph g;
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

void print_graph_stat(Small_Graph::csr_weighted_graph& g){
	 // Optional: Print to verify the size matches the file's metadata!
    std::print("\n\nLoaded {} nodes!", g._node_vec.size());
	std::print("\nLoaded edges num = {}", g._edge_vec.size());

	std::size_t num_edge{};
	for(auto e : g.edges_range()){
		if(e._dest_id != Small_Graph::NULL_EDGE._dest_id){
			num_edge++;
		}
	}
	std::print("\nLoaded real edges num = {}", num_edge);
}

struct BestTargetVisitor {
    static inline std::uint32_t best_target = 0;

    auto find_node(const auto n) const {
        best_target = n;
        return Graph::function::function_flow::iteration_continue;
    }
    auto find_edge([[maybe_unused]] const auto n, [[maybe_unused]] const auto e) const {
        return Graph::function::function_flow::iteration_continue;
    }
    auto end_node([[maybe_unused]] const auto n) const {
        return Graph::function::function_flow::iteration_continue;
    }
};

auto find_best_source_dest(Small_Graph::csr_weighted_graph& g){
	std::uint32_t best_start = 0;
    std::size_t max_edges = 0;
    for(std::uint32_t i = 0; i < g._node_vec.size(); ++i){
        auto edges = g.node_edges_range(i);
        if (edges.size() > max_edges) {
            max_edges = edges.size();
            best_start = i;
        }
    }

    BestTargetVisitor::best_target = best_start;
    g.bfs_loop(best_start, BestTargetVisitor{});

    std::uint32_t best_target = BestTargetVisitor::best_target;
    std::print("\nBest start node: {} with {} edges. Target node: {} (furthest reachable)\n", best_start, max_edges, best_target);

	return std::pair{best_start, best_target};
}

struct StatVisitor {
    static inline std::size_t node_count = 0;
    static inline std::size_t edge_count = 0;

    static void reset() { node_count = 0; edge_count = 0; }

    auto find_node([[maybe_unused]] const auto n) const {
        node_count++;
        return Graph::function::function_flow::iteration_continue;
    }
    auto find_edge([[maybe_unused]] const auto n, [[maybe_unused]] const auto e) const {
        edge_count++;
        return Graph::function::function_flow::iteration_continue;
    }
    auto end_node([[maybe_unused]] const auto n) const {
        return Graph::function::function_flow::iteration_continue;
    }
};

struct CostStatVisitor {
    using cost_type = std::uint32_t;
    static inline std::size_t node_count = 0;
    static inline std::size_t edge_count = 0;
    static inline std::uint32_t total_cost = 0;

    static void reset() { node_count = 0; edge_count = 0; total_cost = 0; }

    auto find_node([[maybe_unused]] const auto n, [[maybe_unused]] const cost_type w) const {
        node_count++;
        return Graph::function::function_flow::iteration_continue;
    }
    auto find_edge([[maybe_unused]] const auto n, [[maybe_unused]] const cost_type w, [[maybe_unused]] const auto e) const {
        edge_count++;
        return Graph::function::function_flow::iteration_continue;
    }
    auto find_edge([[maybe_unused]] const auto n, [[maybe_unused]] const auto e, [[maybe_unused]] const cost_type w) const {
        edge_count++;
        return Graph::function::function_flow::iteration_continue;
    }
    auto cal_cost([[maybe_unused]] const auto n, [[maybe_unused]] const cost_type w, const auto e) const {
        total_cost += e._weight; 
        return w + e._weight;
    }
    auto end_node([[maybe_unused]] const auto n, [[maybe_unused]] const cost_type w) const {
        return Graph::function::function_flow::iteration_continue;
    }
};

struct SearchVisitor {
    static inline std::uint32_t TargetDest = 0;
    static inline std::size_t step_count = 0;
    static inline bool found = false;

    static void reset(std::uint32_t target) { TargetDest = target; step_count = 0; found = false; }

    auto find_node(const auto n) const {
        step_count++;
        if (n == TargetDest) {
            found = true;
            return Graph::function::function_flow::function_return;
        }
        return Graph::function::function_flow::iteration_continue;
    }
    auto find_edge([[maybe_unused]] const auto n, const auto e) const {
        step_count++;
        if (e._dest_id == TargetDest) {
            found = true;
            return Graph::function::function_flow::function_return;
        }
        return Graph::function::function_flow::iteration_continue;
    }
    auto end_node([[maybe_unused]] const auto n) const {
        return Graph::function::function_flow::iteration_continue;
    }
};

struct CostSearchVisitor {
    using cost_type = std::uint32_t;
    static inline std::uint32_t TargetDest = 0;
    static inline std::size_t step_count = 0;
    static inline cost_type reached_cost = 0;
    static inline bool found = false;

    static void reset(std::uint32_t target) { TargetDest = target; step_count = 0; reached_cost = 0; found = false; }

    auto find_node(const auto n, const cost_type w) const {
        step_count++;
        if (n == TargetDest) {
            found = true;
            reached_cost = w;
            return Graph::function::function_flow::function_return;
        }
        return Graph::function::function_flow::iteration_continue;
    }
    auto find_edge([[maybe_unused]] const auto n, const cost_type w, const auto e) const {
        step_count++;
        if (e._dest_id == TargetDest) {
            found = true;
            reached_cost = w + e._weight;
            return Graph::function::function_flow::function_return;
        }
        return Graph::function::function_flow::iteration_continue;
    }
    auto find_edge([[maybe_unused]] const auto n, const auto e, const cost_type w) const {
        step_count++;
        if (e._dest_id == TargetDest) {
            found = true;
            reached_cost = w + e._weight;
            return Graph::function::function_flow::function_return;
        }
        return Graph::function::function_flow::iteration_continue;
    }
    auto cal_cost([[maybe_unused]] const auto n, [[maybe_unused]] const cost_type w, const auto e) const {
        return w + e._weight;
    }
    auto end_node([[maybe_unused]] const auto n, [[maybe_unused]] const cost_type w) const {
        return Graph::function::function_flow::iteration_continue;
    }
};

void dfs_bfs_test(Small_Graph::csr_weighted_graph& g, std::uint32_t start_node, std::uint32_t target_node){
    std::print("\n=== DFS / BFS Tests ===\n");
    
    // DFS Stat Test
    StatVisitor::reset();
    std::print("DFS Loop Stat (Start {}):\n", start_node);
    Timing::measure([&]() {
        g.dfs_loop(start_node, StatVisitor{});
    });
    std::print("DFS Stat: Node count: {}, Edge count: {}\n", StatVisitor::node_count, StatVisitor::edge_count);

    // DFS Search Test
    SearchVisitor::reset(target_node);
    std::print("DFS Loop Search (Start {}, Target {}):\n", start_node, target_node);
    Timing::measure([&]() {
        g.dfs_loop(start_node, SearchVisitor{});
    });
    std::print("DFS Search({}): Found: {}, Steps: {}\n", target_node, SearchVisitor::found, SearchVisitor::step_count);

    // BFS Stat Test
    StatVisitor::reset();
    std::print("BFS Loop Stat (Start {}):\n", start_node);
    Timing::measure([&]() {
        g.bfs_loop(start_node, StatVisitor{});
    });
    std::print("BFS Stat: Node count: {}, Edge count: {}\n", StatVisitor::node_count, StatVisitor::edge_count);

    // BFS Search Test
    SearchVisitor::reset(target_node);
    std::print("BFS Loop Search (Start {}, Target {}):\n", start_node, target_node);
    Timing::measure([&]() {
        g.bfs_loop(start_node, SearchVisitor{});
    });
    std::print("BFS Search({}): Found: {}, Steps: {}\n", target_node, SearchVisitor::found, SearchVisitor::step_count);
}

void ucs_test(Small_Graph::csr_weighted_graph& g, std::uint32_t start_node, std::uint32_t target_node){
    std::print("\n=== UCS Tests ===\n");
    CostStatVisitor::reset();
    std::print("UCS Loop (Start {}):\n", start_node);
    Timing::measure([&]() {
        g.ucs_loop(start_node, CostStatVisitor{});
    });
    std::print("UCS Stat: Node count: {}, Edge count: {}, Total Cost Evaluated: {}\n", 
                 CostStatVisitor::node_count, CostStatVisitor::edge_count, CostStatVisitor::total_cost);

    CostSearchVisitor::reset(target_node);
    std::print("UCS Loop Search for node {}:\n", target_node);
    Timing::measure([&]() {
        g.ucs_loop(start_node, CostSearchVisitor{});
    });
    std::print("UCS Search({}): Found: {}, Steps: {}, Reached Cost: {}\n", 
        target_node, CostSearchVisitor::found, CostSearchVisitor::step_count, CostSearchVisitor::reached_cost);
}

void stat_v(Small_Graph::csr_weighted_graph& g){
	{
		Timing::print_timer _;
		StatVisitor::reset();
		for(int i = 0; i < 10000; ++i){
			g.dfs_loop(4872, StatVisitor{});
		}
	}

	{
		Timing::print_timer _;
		StatVisitor::reset();
		for(int i = 0; i < 10000; ++i){
			g.bfs_loop(4872, StatVisitor{});
		}
	}
}

int main(void){

	auto g = load_graph_from_file("./Dataset/advogato/advogato.edges");
	print_graph_stat(g);

    // dfs_bfs_test(g, 4872, 2086);
    // ucs_test(g, 4872, 2086);

	{
		Timing::print_timer _;
		SearchVisitor::reset(2086);
		for(int i = 0; i < 10000; ++i){
			g.dfs_loop(4872, SearchVisitor{});
		}
	}

	// {
	// 	Timing::print_timer _;
	// 	SearchVisitor::reset(2086);
	// 	for(int i = 0; i < 10000; ++i){
	// 		g.bfs_loop(4872, SearchVisitor{});
	// 	}
	// }

	return 0;
}