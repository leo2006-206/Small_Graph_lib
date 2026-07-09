import std;
import Helper;
// import Graph;
import Small_Graph;

#include <assert.h>

auto create_csr_binary_amazon_2003(){
	auto current_path = std::filesystem::current_path();
	auto data_path = current_path / "Dataset" / "amazon_product_2003/Amazon0601.txt";

	auto opt_graph = SG::utility::load_dir_unwei_dyn_g(data_path);
	// SG::utility::print_g_stat(*opt_graph, 403394, 3387388);
	
	auto opt_csr = SG::csr_graph::make_csr_ref(*opt_graph);
	// SG::utility::print_g_stat(*opt_csr);

	auto bin_path = data_path.parent_path() / "csr_binary_data.bin";
	// std::ofstream create_file(bin_path);

	// opt_csr->save_csr_binary(bin_path);

	// auto opt_csr = SG::csr_graph::load_csr_binary(bin_path);

	return *opt_csr;
}

auto load_print_pokec_social(){
	auto current_path = std::filesystem::current_path();
	auto data_path = current_path / "Dataset" / "pokec_social"/ "soc-pokec-relationships.txt";

	auto opt_graph = SG::utility::load_dir_unwei_dyn_g(data_path, 1);
	SG::utility::print_g_stat(*opt_graph, 1632803, 30622564);
	
	auto opt_csr = SG::csr_graph::make_csr_ref(*opt_graph);
	SG::utility::print_g_stat(*opt_csr);

	return *opt_csr;
}

constexpr int NUM_LOOP = 1;

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

auto dfs_test2(const SG::csr_graph& g){

	SG::utility::mem_distance md{};

	{
		Timing::raii_perf_control pc;
		Timing::print_timer t;
		for(auto l = 0; l < NUM_LOOP; ++l)

		md = SG::dfs_loop_mem_dis(
			g,
			0
		);
	}

	std::println("dfs loop run {}, mean {}, std {}", NUM_LOOP, md.ms.get_mean(), md.ms.get_std());
}

int main(void){
	Debug::clear_log();

	auto csr = create_csr_binary_amazon_2003();
	// auto csr = load_print_pokec_social();

	// dyn_test(g);
	// csr_test(csr);
	// csr_test2(csr);
	// dfs_test(csr);
	dfs_test2(csr);

	return 0;
}