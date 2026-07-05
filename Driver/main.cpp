import std;
import Helper;
// import Graph;
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

		SG::node_id_t sour, dist;

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

auto csr_load(SG::dyn_graph in_graph){
	using namespace SG;

	auto g = *SG::csr_graph::make_csr(in_graph);
	auto size = g.node_vec_.size() * sizeof(csr_graph::edge_t) + g.edge_vec_.size() * sizeof(csr_graph::node_t);
	std::println("total size = {}", size);

	return g;
}

constexpr int NUM_LOOP = 100;

auto dyn_test(const SG::dyn_graph& g){
	std::size_t sum{};
	
	{
		Timing::raii_perf_control pc;
		Timing::print_timer t;
		for(auto l = 0; l < NUM_LOOP; ++l)

		for(auto& [key, node] : g.node_table_){
			for(auto dist : node.edges_span()){
				sum += node.source_id_;
				sum ^= dist;
			}
		}
	
	}

	std::println("{}", sum);
}

auto csr_test(const SG::csr_graph& g){
	std::size_t sum{};
	
	{
		Timing::raii_perf_control pc;
		Timing::print_timer t;
		for(auto l = 0; l < NUM_LOOP; ++l)

		for(auto ae : g.edges_range()){
			sum += ae.source;
			sum ^= ae.dist;
		}
	
	}

	std::println("{}", sum);
}

auto csr_test2(const SG::csr_graph& g){
	std::size_t sum{};
	
	{
		Timing::raii_perf_control pc;
		Timing::print_timer t;
		for(auto l = 0; l < NUM_LOOP; ++l)

		g.for_each_edge([&sum](SG::alone_edge e){
			sum += e.source;
			sum ^= e.dist;
		});
	
	}

	std::println("{}", sum);
}

auto dfs_test(const SG::csr_graph& g){
	std::size_t num_edge{};

	{
		Timing::raii_perf_control pc;
		Timing::print_timer t;
		for(auto l = 0; l < NUM_LOOP; ++l)

		SG::dfs_loop(
			g,
			[&num_edge](SG::alone_edge){num_edge++;},
			0
		);
	}

	std::println("dfs loop run {}, result {}", NUM_LOOP, num_edge);
}

int main(void){
	Debug::clear_log();

	auto current_path = std::filesystem::current_path();
	auto data_path = current_path / "Dataset" / "amazon_product_2003/Amazon0601.txt";

	auto g = load_graph(data_path);
	auto csr = csr_load(g);

	// dyn_test(g);
	// csr_test(csr);
	// csr_test2(csr);
	dfs_test(csr);

	return 0;
}