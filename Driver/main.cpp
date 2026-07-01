import std;
import Helper;
import Graph;
import Small_Graph;

#include <assert.h>

auto load_graph(const std::filesystem::path& file_path){
	std::cout<< file_path << "\n";

	SG::dyn_graph g;
	std::size_t success_count{};

	std::ifstream file(file_path);
	std::string line;

	while(std::getline(file, line)){
		if(line.empty() || line[0] == '#'){
			continue;
		}

		std::ranges::replace(line, ',', ' ');

		std::istringstream iss{line};

		SG::dynamic_graph::node_id_t sour, dist;

		if(iss >> sour >> dist){
			success_count += g.insert_edge(sour, dist);
		}
	}

	std::println("Original |V| = {}, |E| = {}", 403394, 3387388);
	std::println("Graph |V| = {}, |E| = {}", g.node_table_.size(), success_count);
	std::println("invalid v = {}", g.checking_edge_integrity().size());
	std::println("packing vec size = {}", g.packing_node_id_vec().size());

	return g;
}

int main(void){
	Debug::clear_log();

	auto current_path = std::filesystem::current_path();
	auto data_path = current_path / "Dataset" / "amazon_product_2003/Amazon0601.txt";

	auto g = load_graph(data_path);

	return 0;
}