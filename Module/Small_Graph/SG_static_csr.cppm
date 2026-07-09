module;

export module Small_Graph:static_graph;

import :core;
import :dynamic_graph;
import std;

export namespace SG{

constexpr bool IS_csr_node_contains_checking = true;

struct csr_graph{
	// whole class is const only;
	// currently unweighted directed graph only
	using csr_node_id_t		= node_id_t;
	using csr_edge_offset_t	= edge_offset_t;
	
	const std::vector<csr_edge_offset_t>	node_vec_;
	const std::vector<csr_node_id_t>		edge_vec_;
	
	csr_graph() = delete;
	
	//make_csr is the constructor, dyn_graph copy for safety
	constexpr static			std::optional<csr_graph>		make_csr(dyn_graph in_graph);
	constexpr static			std::optional<csr_graph>		make_csr_ref(dyn_graph& in_graph);

	constexpr static			std::optional<csr_graph>		load_csr_binary(const std::filesystem::path& file_path);

	constexpr		 			std::size_t						save_csr_binary(const std::filesystem::path& file_path) const;

	[[nodiscard]] constexpr		bool							node_contains(csr_node_id_t id) const;
	[[nodiscard]] constexpr		bool							edge_contains(alone_edge in_edge) const;

	[[nodiscard]] constexpr		std::size_t						node_size() const;
	[[nodiscard]] constexpr		std::size_t						edge_size() const;

	[[nodiscard]] constexpr		std::optional<std::size_t>		node_degree(csr_node_id_t id) const;

	[[nodiscard]] constexpr 	std::span<const csr_node_id_t>	node_edges_range(csr_node_id_t id) const;

	[[nodiscard]] constexpr		auto/*range<csr_node_id_t>*/	nodes_range() const;
	[[nodiscard]] constexpr		auto/*range<alone_edge>*/		edges_range() const;
	
	constexpr					void							for_each_edge(
		std::invocable<alone_edge> auto&& functor) const;

	private:
	csr_graph(
		std::vector<csr_edge_offset_t>&&	in_node_vec, 
		std::vector<csr_node_id_t>&&		in_edge_vec 
	): 
	node_vec_(std::move(in_node_vec)),
	edge_vec_(std::move(in_edge_vec)){}
};

}

namespace SG {
namespace stdr	= std::ranges;
namespace stdrv	= std::ranges::views;

constexpr		std::optional<csr_graph>		csr_graph::make_csr(dyn_graph in_graph){
	return make_csr_ref(in_graph);
}

constexpr		std::optional<csr_graph>		csr_graph::make_csr_ref(dyn_graph& in_graph){
	if(in_graph.node_table_.empty()){
		return std::nullopt;
	}
	if(in_graph.checking_edge_integrity().size() > 0){
		return std::nullopt;
	}
	[[maybe_unused]]auto packing_vec = in_graph.packing_node_id_vec();
	for(auto& [key, node] : in_graph.node_table_){
		node.sort_edges();
	}

	const auto& ready_graph = in_graph;
	const std::size_t csr_node_id_table_size = ready_graph.node_table_.size();

	std::vector<csr_edge_offset_t> node_vec;
	std::vector<csr_node_id_t> edge_vec;

	node_vec.reserve(csr_node_id_table_size + 1);
	edge_vec.reserve(csr_node_id_table_size * 4);

	csr_edge_offset_t current_count{0};
	for(auto packed_id : stdrv::iota(0uz, csr_node_id_table_size)){
		node_vec.emplace_back(current_count);

		for(auto dist : ready_graph.node_edges_span(static_cast<node_id_t>(packed_id))){
			edge_vec.emplace_back(dist);
			current_count++;
		}
	}

	node_vec.emplace_back(current_count);

	node_vec.shrink_to_fit();
	edge_vec.shrink_to_fit();

	return csr_graph(
		std::move(node_vec),
		std::move(edge_vec)
	);
}

constexpr		std::optional<csr_graph>		csr_graph::load_csr_binary(const std::filesystem::path& file_path){
	if(std::filesystem::is_regular_file(file_path) == false){
		return std::nullopt;
	}

	std::ifstream in(file_path, std::ios::binary);
	
	if(!in){
		return std::nullopt;
	}

	std::size_t num_node{};
	std::size_t num_edge{};

	in.read(reinterpret_cast<char*>(&num_node), sizeof(std::size_t));
	in.read(reinterpret_cast<char*>(&num_edge), sizeof(std::size_t));

	std::vector<csr_edge_offset_t>	node_vec;
	std::vector<csr_node_id_t>		edge_vec;
	node_vec.resize(num_node);
	edge_vec.resize(num_edge);

	in.read(
		reinterpret_cast<char*>(node_vec.data()), 
		signed(num_node * sizeof(csr_edge_offset_t))
	);
	in.read(
		reinterpret_cast<char*>(edge_vec.data()), 
		signed(num_edge * sizeof(csr_node_id_t))
	);

	return csr_graph{std::move(node_vec), std::move(edge_vec)};
}

constexpr		std::size_t						csr_graph::save_csr_binary(const std::filesystem::path& file_path)const{
	if(std::filesystem::is_regular_file(file_path) == false){
		return 0;
	}
	std::ofstream out(file_path, std::ios::binary);
	if(!out){
		return 0;
	}

	std::size_t saved_bytes{};

	std::size_t num_node = node_size();
	std::size_t num_edge = edge_size();

	auto write_counted = [&](const std::ofstream::char_type* char_ptr, std::streamsize len){
		out.write(char_ptr, len);
		saved_bytes += static_cast<std::size_t>(len);
	};

	write_counted(reinterpret_cast<const char*>(&num_node), sizeof(std::size_t));
	write_counted(reinterpret_cast<const char*>(&num_edge), sizeof(std::size_t));

	write_counted(
		reinterpret_cast<const char*>(node_vec_.data()), 
		signed(num_node * sizeof(csr_edge_offset_t))
	);
	write_counted(
		reinterpret_cast<const char*>(edge_vec_.data()), 
		signed(num_edge * sizeof(csr_node_id_t))
	);

	return saved_bytes;
}

constexpr		bool							csr_graph::node_contains(csr_node_id_t id) const{
	if(id < (node_vec_.size() - 1)){
		return true;
	}
	return false;
}
constexpr		bool							csr_graph::edge_contains(alone_edge in_edge) const{
	if constexpr (IS_csr_node_contains_checking){
	
		if(
			node_contains(in_edge.source) == false ||
			node_contains(in_edge.dist) == false
		){
			return false;
		}

	}

	return stdr::binary_search(
		node_edges_range(in_edge.source),
		in_edge.dist
	);
}

constexpr		std::size_t						csr_graph::node_size() const{
	return node_vec_.size() - 1;
}
constexpr		std::size_t						csr_graph::edge_size() const{
	return edge_vec_.size();
}
constexpr		std::optional<std::size_t>		csr_graph::node_degree(csr_node_id_t id) const{
	if constexpr (IS_csr_node_contains_checking){

		if(node_contains(id) == false){
			return std::nullopt;
		}

	}

	return node_vec_[id+1] - node_vec_[id];
}

constexpr 		std::span<const csr_graph::csr_node_id_t>			csr_graph::node_edges_range(csr_node_id_t id) const{
	if constexpr (IS_csr_node_contains_checking){
	
		if(node_contains(id) == false){
			return {};
		}

	}

	auto span = std::span{edge_vec_};
	// return span.subspan(node_vec_[id], *node_degree(id));
	return span.subspan(node_vec_[id], node_vec_[id+1] - node_vec_[id]);
}

constexpr		auto/*range<const csr_node_id_t>*/		csr_graph::nodes_range() const{
	return stdrv::iota(
		static_cast<csr_node_id_t>(0), 
		static_cast<csr_node_id_t>(node_vec_.size() - 1)
	);
}
constexpr		auto/*range<alone_edge>*/		csr_graph::edges_range() const{	
	return nodes_range() 
		| stdrv::transform(
			[this](csr_node_id_t source){
			return node_edges_range(source)
				| stdrv::transform(
					[source](csr_node_id_t dist){
						return alone_edge(source, dist);
					}
				);
			}
		)
		| stdrv::join;
}

constexpr					void				csr_graph::for_each_edge(
	std::invocable<alone_edge> auto&& functor
) const{
	const auto num_node =static_cast<csr_node_id_t>( node_vec_.size() - 1 );
	for(csr_node_id_t source{0}; source < num_node; ++source){
		for(csr_node_id_t dist : node_edges_range(source)){
			functor(alone_edge{source, dist});
		}
	}
}

}