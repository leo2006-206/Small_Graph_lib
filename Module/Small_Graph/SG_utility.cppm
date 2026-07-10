module;

#include <cassert>

export module Small_Graph:utility;

import std;
import :dynamic_graph;
import :static_graph;

export namespace SG::utility{

std::string						nice_memory_str(const std::size_t num_bytes);

std::optional<SG::dyn_graph>	load_dir_unwei_dyn_g(
	const std::filesystem::path& file_path,
	SG::node_id_t id_offset = node_id_t{0}
);

std::optional<SG::csr_graph>	load_dir_unwei_csr_g(
	const std::filesystem::path& file_path,
	SG::node_id_t id_offset = node_id_t{0}
);

void							print_g_stat(
	const SG::dyn_graph&	graph,
	std::size_t		num_node = 0,
	std::size_t		num_edge = 0
);

void							print_g_stat(const SG::csr_graph&	graph);

}

// function implementation start here
namespace SG::utility{

std::string nice_memory_str(std::size_t num_bytes) {
	if (num_bytes >= 1'000'000'000) {
		// Greater than 1 billion bytes? Print as GB and GiB
		auto gb  = static_cast<double>(num_bytes) / 1'000'000'000.0;
		auto gib = static_cast<double>(num_bytes) / 1'073'741'824.0; // 1024^3
		return std::format("{:.2f} GB ({:.2f} GiB)", gb, gib);
		
	} else if (num_bytes >= 1'000'000) {
		// Greater than 1 million bytes? Print as MB and MiB
		auto mb  = static_cast<double>(num_bytes) / 1'000'000.0;
		auto mib = static_cast<double>(num_bytes) / 1'048'576.0;     // 1024^2
		return std::format("{:.2f} MB ({:.2f} MiB)", mb, mib);
		
	} else if (num_bytes >= 1'000) {
		// Greater than 1 thousand bytes? Print as KB and KiB
		auto kb  = static_cast<double>(num_bytes) / 1'000.0;
		auto kib = static_cast<double>(num_bytes) / 1024.0;          // 1024^1
		return std::format("{:.2f} KB ({:.2f} KiB)", kb, kib);
		
	} else {
		// Less than 1000 bytes? No decimals needed
		return std::format("{} Bytes", num_bytes);
	}
}

std::optional<SG::dyn_graph>	load_dir_unwei_dyn_g(
	const std::filesystem::path& file_path,
	SG::node_id_t id_offset
){
	SG::dyn_graph g;

	if(std::filesystem::is_regular_file(file_path) == false){
		return std::nullopt;
	}

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

			assert(
				std::min(sour, dist) >= id_offset &&
				"id_offset must <= Min of all node ids, where no underflow after `id - offset`"
			);

			g.insert_edge(sour - id_offset, dist - id_offset);
		}
	}

	return g;
}

std::optional<SG::csr_graph>	load_dir_unwei_csr_g(
	const std::filesystem::path& file_path,
	SG::node_id_t id_offset
){
	std::optional<SG::dyn_graph> opt_g = load_dir_unwei_dyn_g(file_path, id_offset);
	
	if(opt_g.has_value()){
		return SG::csr_graph::make_csr_ref(*opt_g);
	}
	return std::nullopt;
}

void							print_g_stat(
	const SG::dyn_graph&	graph,
	std::size_t		num_node,
	std::size_t		num_edge
){

	if(num_node == 0 && num_edge == 0){
		std::println("No |V| & |E| from original dataset");
	}
	else{
		std::println("Original\t|V| = {},\t|E| = {}", num_node, num_edge);
	}

	std::println("Dyn_graph\t|V| = {}, \t|E| = {}", graph.node_size(), graph.edge_size());
	std::println("Integrity check, Dangling |E| = {}", graph.checking_edge_integrity().size());
	std::println("Need node id packing = {}", graph.need_packing_node_id());

	struct mock_map_node{
		void*		next;
		std::size_t	hash;
		std::pair<
			const SG::dyn_graph::node_table_t::key_type,
			SG::dyn_graph::node_table_t::value_type
		>			key_value;
	};

	std::size_t bucket_byte = graph.node_table_.bucket_count() * sizeof(void*);
	std::size_t node_byte = graph.node_size() * sizeof(mock_map_node);

	std::println("Dyn_graph hash table");
	std::println("\tsizeof( bucket_array )\t~ {}", nice_memory_str(bucket_byte));
	std::println("\tsizeof( table_node )\t~ {}" , nice_memory_str(node_byte));

	std::size_t dyn_nodes_byte{};
	for(const auto& [key, node] : graph.node_table_){
		dyn_nodes_byte += node.edge_vec_.capacity() * sizeof(SG::dyn_node::edge_vec_t::value_type);
	}
	std::println("\tsizeof( sum dyn_node )\t= {}", nice_memory_str(dyn_nodes_byte));
	std::println(
		"\tSum sizeof( dyn_graph)\t~ {}",
		nice_memory_str(bucket_byte + node_byte + dyn_nodes_byte)
	);
}

void							print_g_stat(const SG::csr_graph&	graph){
	std::println("CSR_graph\t|V| = {}, \t|E| = {}", graph.node_size(), graph.edge_size());
	
	auto node_byte = (graph.node_size() + 1) * sizeof(SG::csr_graph::csr_edge_offset_t);
	auto edge_byte = graph.edge_size() * sizeof(SG::csr_graph::csr_node_id_t);

	std::println("\tsizeof( node_vector )\t= {}", nice_memory_str(node_byte));
	std::println("\tsizeof( edge_vector )\t= {}", nice_memory_str(edge_byte));
	std::println("\tSum sizeof( CSR_graph )\t= {}", nice_memory_str(node_byte + edge_byte));
}

}

// struct implementation start here
namespace SG::utility{
}