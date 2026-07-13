import std;
import Helper;
// import Graph;
import Small_Graph;

#include <assert.h>

auto create_csr_binary(const std::filesystem::path& file_path, const std::string_view bin_file){
	using namespace SG;
	auto binary_path = file_path.parent_path() / bin_file;
	if(std::filesystem::exists(binary_path)){
		if(std::filesystem::file_size(binary_path)){
			std::println("graph binary `{}` exists", file_path.parent_path().stem().string());
			return;
		}
	}
	else{
		std::ofstream _(binary_path, std::ios::binary);
		std::println("graph binary `{}` not exist, created new binary file", file_path.parent_path().stem().string());
	}

	Timing::print_timer _("Creating csr binary");

	auto opt_graph = utility::load_dir_unwei_csr_g(file_path);
	if(opt_graph.has_value() == false){
		std::println("graph loading failed with {}, exit", file_path.parent_path().stem().string());
		std::exit(0);
	}
	auto written_bytes = opt_graph->save_csr_binary(binary_path);
	utility::print_g_stat(*opt_graph);
	std::println("written binary bytes = {}, filesystem::file_size = {}", 
		written_bytes, 
		std::filesystem::file_size(binary_path)
	);
}

auto load_csr_binary(const std::filesystem::path& file_path, const std::string_view bin_file){
	using namespace SG;
	auto binary_path = file_path.parent_path() / bin_file;
	auto opt_graph = csr_graph::load_csr_binary(binary_path);
	if(opt_graph.has_value()){
		std::println("loaded graph {}", file_path.parent_path().stem().string());
		utility::print_g_stat(*opt_graph);
		return *opt_graph;
	}
	else{
		std::println("failed to load graph, path = {}", binary_path.string());
		std::exit(0);
	}
}

auto csr_test(const SG::csr_graph& g, int NUM_LOOP){
	std::size_t sum{};
	
	{
		Timing::raii_perf_control pc;
		Timing::print_timer t;
		for(int l = 0; l < NUM_LOOP; ++l)

		for(auto ae : g.edges_range()){
			sum += ae.source;
			sum ^= ae.dist;
		}
	
	}

	std::println("{}", sum);
}

auto csr_test2(const SG::csr_graph& g, int NUM_LOOP){
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

auto dfs_test(const SG::csr_graph& g, int NUM_LOOP){
	std::size_t num_edge{0};

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

	std::println("dfs loop run {}, num looped edge {}", NUM_LOOP, num_edge);
}

auto dfs_test_mem(const SG::csr_graph& g, int NUM_LOOP){

	SG::mem_distance md{};

	{
		Timing::raii_perf_control pc;
		Timing::print_timer t;
		for(auto l = 0; l < NUM_LOOP; ++l)

		md = SG::dfs_loop_mem_dis(
			g,
			0
		);
	}

	std::println("dfs loop run {}, count {}, mean {}, std {}", 
		NUM_LOOP,md.ms.count, md.ms.get_mean(), md.ms.get_std());
}

auto dfs_all_test(const SG::csr_graph& g, int NUM_LOOP){
	std::size_t num_edge{};

	{
		Timing::raii_perf_control pc;
		Timing::print_timer t;
		for(auto l = 0; l < NUM_LOOP; ++l)

		SG::dfs_loop_all(
			g,
			[&num_edge](SG::alone_edge){num_edge++;},
			0
		);
	}

	std::println("dfs_loop_all run {}, num looped edge {}", NUM_LOOP, num_edge);
}

auto bfs_test_mem(const SG::csr_graph& g, int NUM_LOOP){

	SG::mem_distance md{};

	{
		Timing::raii_perf_control pc;
		Timing::print_timer t;
		for(auto l = 0; l < NUM_LOOP; ++l)

		md = SG::bfs_loop_mem_dis(
			g,
			0
		);
	}

	std::println("bfs loop run {}, count {}, mean {}, std {}", 
		NUM_LOOP,md.ms.count, md.ms.get_mean(), md.ms.get_std());
}

int main(void){

	const auto binary_file_name = std::string_view("csr_binary_data.bin");
	auto current_path = std::filesystem::current_path();
	
	auto amazon_path = current_path / "Dataset/amazon_product_2003/Amazon0601.txt";
	auto pokec_social = current_path / "Dataset/pokec_social/soc-pokec-relationships.txt";
	auto uk2002 = current_path / "Dataset/uk_2002/uk_2002_edgelist.txt";

	create_csr_binary(amazon_path, binary_file_name);
	create_csr_binary(pokec_social, binary_file_name);
	create_csr_binary(uk2002, binary_file_name);

	// auto csr = load_csr_binary(amazon_path, binary_file_name);
	// auto csr = load_csr_binary(pokec_social, binary_file_name);
	auto csr = load_csr_binary(uk2002, binary_file_name);

	auto benckmark= [](const SG::csr_graph& csr){
		const int NUM_LOOP = 1;

		// csr_test(csr, NUM_LOOP);
		// csr_test2(csr, NUM_LOOP);
		// dfs_test(csr, NUM_LOOP);
		// dfs_test_mem(csr, NUM_LOOP);
		// dfs_all_test(csr, NUM_LOOP);
		bfs_test_mem(csr, NUM_LOOP);

	};

	benckmark(csr);

	return 0;
}