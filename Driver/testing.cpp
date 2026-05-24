import std;
import Graph;

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

int main(void){

	auto g = load_graph_from_file("./Dataset/advogato/advogato.edges");
	print_graph_stat(g);

	return 0;
}