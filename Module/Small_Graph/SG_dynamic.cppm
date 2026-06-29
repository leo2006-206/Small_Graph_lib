module;

export module Small_Graph:dynamic_graph;

import std;

namespace SG::dynamic_graph{

template<typename range_t, typename value_t>
concept input_range_convertible = requires (range_t, value_t){
	requires std::ranges::input_range<range_t>;
	requires std::is_convertible_v<std::ranges::range_reference_t<range_t>, value_t>;
};

}


export namespace SG::dynamic_graph{

//Currently only targeting unweighted directed graph

using node_id_t = std::uint64_t;

struct	alone_edge{
	node_id_t	source;
	node_id_t	dist;

	explicit constexpr alone_edge(node_id_t in_source, node_id_t in_dist)
	: source(in_source), dist(in_dist){}
};

struct	dyn_node{
	using edge_vec_t = std::vector<node_id_t>;

	node_id_t	source_id_;
	edge_vec_t	edge_vec_;

	dyn_node() = delete;

	explicit constexpr dyn_node(node_id_t in_source, std::size_t reserve_size = 32)
	: source_id_(in_source), edge_vec_(){
		edge_vec_.reserve(reserve_size);
	}

	[[nodiscard]] constexpr		bool						operator==(const dyn_node& other) const{
		return (source_id_ == other.source_id_);
	}

	[[nodiscard]] constexpr		bool						edge_contains(node_id_t in_dist) const;

	[[nodiscard]] constexpr		std::span<node_id_t>		edges_span();
	[[nodiscard]] constexpr		std::span<const node_id_t>	edges_span() const;

	[[nodiscard]] constexpr		auto/*range<alone_edge>*/	alone_edges_range() const;

	[[nodiscard]] constexpr		bool						insert_edge(node_id_t in_dist);
	constexpr					bool						try_insert_edge(node_id_t in_dist);
	constexpr					std::size_t					insert_range_edges(/*const*/input_range_convertible<node_id_t> auto&& in_range);
	[[nodiscard]] constexpr		std::vector<node_id_t>		insert_range_edges_vec(
		/*const*/input_range_convertible<node_id_t> auto&& in_range, std::size_t reserve_size
	);

	[[nodiscard]] constexpr		bool						remove_edge(node_id_t in_dist);
	constexpr					bool						try_remove_edge(node_id_t in_dist);
	constexpr					std::size_t					remove_range_edges(input_range_convertible<node_id_t> auto&& in_range);
	[[nodiscard]] constexpr		std::vector<node_id_t>		remove_range_edges_vec(
		/*const*/input_range_convertible<node_id_t> auto&& in_range, std::size_t reserve_size
	);

	void													sort_edges();
};

struct	dyn_graph{


};

}


// implementation start here
namespace SG::dynamic_graph{

namespace	stdr	= std::ranges;
namespace	stdrv	= std::ranges::views;

constexpr		bool						dyn_node::edge_contains(node_id_t in_dist) const{
	if(in_dist == source_id_){return false;}
	return stdr::find(edge_vec_, in_dist) != edge_vec_.end();
}

constexpr		std::span<node_id_t>		dyn_node::edges_span(){
	return {edge_vec_};
}
constexpr		std::span<const node_id_t>	dyn_node::edges_span() const{
	return {edge_vec_};
}

constexpr		auto/*range<alone_edge>*/	dyn_node::alone_edges_range() const{
	auto make_alone = [this](node_id_t dist){
		return alone_edge{source_id_, dist};
	};

	return edge_vec_ | stdrv::transform(make_alone);
}

constexpr		bool						dyn_node::insert_edge(node_id_t in_dist){
	if(in_dist == source_id_){return false;}
	if(edge_contains(in_dist)){
		return false;
	}
	edge_vec_.emplace_back(in_dist);
	return true;
}
constexpr		bool						dyn_node::try_insert_edge(node_id_t in_dist){
	if(in_dist == source_id_){return false;}
	if(edge_contains(in_dist)){
		return false;
	}
	edge_vec_.emplace_back(in_dist);
	return true;
}
constexpr		std::size_t					dyn_node::insert_range_edges(input_range_convertible<node_id_t> auto&& in_range){
	std::size_t insert_failed_count{};

	for(node_id_t dist : in_range){
		if(insert_edge(dist) == false){
			insert_failed_count++;
		}
	}

	return insert_failed_count;
}
constexpr		std::vector<node_id_t>		dyn_node::insert_range_edges_vec(
	/*const*/input_range_convertible<node_id_t> auto&& in_range, std::size_t reserve_size
){
	std::vector<node_id_t> failed_dist;
	failed_dist.reserve(reserve_size);

	for(node_id_t dist : in_range){
		if(insert_edge(dist) == false){
			failed_dist.emplace_back(dist);
		}
	}

	return failed_dist;
}

constexpr		bool						dyn_node::remove_edge(node_id_t in_dist){
	if(in_dist == source_id_){return false;}
	auto it = stdr::find(edge_vec_, in_dist);
	if(it == edge_vec_.end()){
		return false;
	}
	edge_vec_.erase(it);
	return true;
}
constexpr		bool						dyn_node::try_remove_edge(node_id_t in_dist){
	if(in_dist == source_id_){return false;}
	auto it = stdr::find(edge_vec_, in_dist);
	if(it == edge_vec_.end()){
		return false;
	}
	edge_vec_.erase(it);
	return true;
}
constexpr		std::size_t					dyn_node::remove_range_edges(input_range_convertible<node_id_t> auto&& in_range){
	std::size_t remove_failed_count{};

	for(node_id_t dist : in_range){
		if(remove_edge(dist) == false){
			remove_failed_count++;
		}
	}

	return remove_failed_count;
}
constexpr		std::vector<node_id_t>		dyn_node::remove_range_edges_vec(
	/*const*/input_range_convertible<node_id_t> auto&& in_range, std::size_t reserve_size
){
	std::vector<node_id_t> failed_dist;
	failed_dist.reserve(reserve_size);

	for(node_id_t dist : in_range){
		if(remove_edge(dist) == false){
			failed_dist.emplace_back(dist);
		}
	}

	return failed_dist;
}


void										dyn_node::sort_edges(){
	stdr::sort(edge_vec_);
}
	
}

// std extra formatter and hash here
template <>
struct std::formatter<SG::dynamic_graph::alone_edge> : std::formatter<std::string>{

	auto format(const SG::dynamic_graph::alone_edge& e, std::format_context& ctx) const {
        // std::format_to writes directly to the output buffer
        return std::format_to(ctx.out(), "({} -> {}) ", e.source, e.dist);
    }

};