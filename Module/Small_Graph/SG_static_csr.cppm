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
	using node_t	= csr_node_t;
	using edge_t	= csr_edge_t;
	
	const std::vector<edge_t>	node_vec_;
	const std::vector<node_t>	edge_vec_;
	
	csr_graph() = delete;
	
	//make_csr is the constructor, dyn_graph copy for safety
	constexpr static			std::optional<csr_graph>		make_csr(dyn_graph in_graph);
	constexpr static			std::optional<csr_graph>		make_csr_ref(dyn_graph& in_graph);

	[[nodiscard]] constexpr		bool							node_contains(node_t id) const;
	[[nodiscard]] constexpr		bool							edge_contains(alone_edge in_edge) const;

	[[nodiscard]] constexpr		std::optional<std::size_t>		node_degree(node_t id) const;

	[[nodiscard]] constexpr 	std::span<const node_t>			node_edges_range(node_t id) const;

	[[nodiscard]] constexpr		auto/*range<const node_t>*/		nodes_range() const;
	[[nodiscard]] constexpr		auto/*range<alone_edge>*/		edges_range() const;
	
	constexpr					void							for_each_edge(
		std::invocable<alone_edge> auto&& functor) const;

	private:
	csr_graph(
		std::vector<edge_t>&& in_node_vec, 
		std::vector<node_t>&& in_edge_vec 
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
	const std::size_t node_table_size = ready_graph.node_table_.size();

	std::vector<edge_t> node_vec;
	std::vector<node_t> edge_vec;

	node_vec.reserve(node_table_size + 1);
	edge_vec.reserve(node_table_size * 4);

	edge_t current_count{0};
	for(auto packed_id : stdrv::iota(0uz, node_table_size)){
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


constexpr		bool							csr_graph::node_contains(node_t id) const{
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

constexpr		std::optional<std::size_t>						csr_graph::node_degree(node_t id) const{
	if constexpr (IS_csr_node_contains_checking){

		if(node_contains(id) == false){
			return std::nullopt;
		}

	}

	return node_vec_[id+1] - node_vec_[id];
}

constexpr 		std::span<const csr_graph::node_t>			csr_graph::node_edges_range(node_t id) const{
	if constexpr (IS_csr_node_contains_checking){
	
		if(node_contains(id) == false){
			return {};
		}

	}

	auto span = std::span{edge_vec_};
	return span.subspan(node_vec_[id], node_vec_[id+1] - node_vec_[id]);
}

constexpr		auto/*range<const node_t>*/		csr_graph::nodes_range() const{
	return stdrv::iota(
		static_cast<node_t>(0), 
		static_cast<node_t>(node_vec_.size() - 1)
	);
}
constexpr		auto/*range<alone_edge>*/		csr_graph::edges_range() const{	
	return nodes_range() 
		| stdrv::transform(
			[this](node_t source){
			return node_edges_range(source)
				| stdrv::transform(
					[source](node_t dist){
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
	const auto num_node =static_cast<node_t>( node_vec_.size() - 1 );
	for(node_t source{0}; source < num_node; ++source){
		for(node_t dist : node_edges_range(source)){
			functor(alone_edge{source, dist});
		}
	}
}

}