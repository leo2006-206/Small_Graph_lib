module;

export module Small_Graph:dynamic_graph;

import std;
import :core;

export namespace SG{

//Currently only targeting unweighted directed graph


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
	[[nodiscard]] constexpr		std::size_t					out_degree() const;

	[[nodiscard]] constexpr		std::span<node_id_t>		edges_span();
	[[nodiscard]] constexpr		std::span<const node_id_t>	edges_span() const;

	//return range<alone_edge>, alone_edge{source_id_, dist} is new created, not proxy of original dist
	[[nodiscard]] constexpr		auto/*range<alone_edge>*/	alone_edges_range() const;

	[[nodiscard]] constexpr		bool						insert_edge(node_id_t in_dist);
	constexpr					bool						try_insert_edge(node_id_t in_dist);
	constexpr					std::size_t/*failed_count*/	insert_range_edges(/*const*/input_range_convertible<node_id_t> auto&& in_range);
	[[nodiscard]] constexpr		std::vector<node_id_t>		insert_range_edges_vec(
		/*const*/input_range_convertible<node_id_t> auto&& in_range, std::size_t reserve_size
	);							/*return vector{failed_edges}*/

	[[nodiscard]] constexpr		bool						remove_edge(node_id_t in_dist);
	constexpr					bool						try_remove_edge(node_id_t in_dist);
	constexpr					std::size_t/*failed_count*/	remove_range_edges(input_range_convertible<node_id_t> auto&& in_range);
	[[nodiscard]] constexpr		std::vector<node_id_t>		remove_range_edges_vec(
		/*const*/input_range_convertible<node_id_t> auto&& in_range, std::size_t reserve_size
	);							/*return vector{failed_edges}*/

	void													sort_edges();
};

struct	dyn_graph{
	using node_table_t = std::unordered_map<
		node_id_t,	//key
		dyn_node,	//value
		std::hash<node_id_t>,
		std::equal_to<node_id_t>,
		std::allocator<std::pair<const node_id_t, dyn_node>>
	>;

	using iter_t 		= node_table_t::iterator;
	using const_iter_t	= node_table_t::const_iterator;

	using const_id_map_t= const std::unordered_map<node_id_t, node_id_t>;

	node_table_t	node_table_;

	explicit constexpr dyn_graph(std::size_t reserve_size = 64)
	: node_table_(){
		node_table_.reserve(reserve_size);
	}

	[[nodiscard]] constexpr		std::size_t/*O( 1 )*/		node_size() const;
	[[nodiscard]] constexpr		std::size_t/*O( |E| )*/		edge_size() const;

	[[nodiscard]] constexpr		bool						node_contains(node_id_t in_id) const;
	[[nodiscard]] constexpr		bool						edge_contains(alone_edge in_edge) const;
	[[nodiscard]] constexpr		bool						edge_contains(node_id_t source, node_id_t dist) const;

	[[nodiscard]] constexpr		std::span<const node_id_t>	node_edges_span(node_id_t in_id) const;

	[[nodiscard]] constexpr		iter_t						find_node(node_id_t in_id);
	[[nodiscard]] constexpr		const_iter_t				find_node(node_id_t in_id) const;

	constexpr		bool									insert_edge(alone_edge in_edge);
	constexpr		bool									insert_edge(node_id_t source, node_id_t dist);
	constexpr		bool									remove_edge(alone_edge in_edge);


	constexpr					std::size_t/*failed count*/	insert_range(input_range_convertible<alone_edge> auto&& in_range);
	constexpr					std::size_t/*failed count*/	remove_range(input_range_convertible<alone_edge> auto&& in_range);
	
	[[nodiscard]] constexpr		std::vector<alone_edge>		checking_edge_integrity(std::size_t reserve_size = 0) const;
	
	[[nodiscard]] constexpr		bool						need_packing_node_id() const;
	[[nodiscard]] constexpr		std::vector<node_id_t>		need_packing_node_id_vec() const;

	[[nodiscard]] constexpr		const_id_map_t				packing_node_id_map();
	[[nodiscard]] constexpr		std::vector<node_id_t>		packing_node_id_vec();
};

}

// dyn_node implementation start here
namespace SG{

namespace	stdr	= std::ranges;
namespace	stdrv	= std::ranges::views;

constexpr		bool						dyn_node::edge_contains(node_id_t in_dist) const{
	if constexpr (IS_simple_graph == true){
		if(in_dist == source_id_){
			return false;
		}
	}

	return stdr::find(edge_vec_, in_dist) != edge_vec_.end();
}
constexpr		std::size_t					dyn_node::out_degree() const{
	return edge_vec_.size();
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
	if constexpr (IS_simple_graph == true){
		if(in_dist == source_id_){
			return false;
		}
	}

	if(edge_contains(in_dist)){
		return false;
	}
	edge_vec_.emplace_back(in_dist);
	return true;
}
constexpr		bool						dyn_node::try_insert_edge(node_id_t in_dist){
	if constexpr (IS_simple_graph == true){
		if(in_dist == source_id_){
			return false;
		}
	}

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
	if constexpr (IS_simple_graph == true){
		if(in_dist == source_id_){
			return false;
		}
	}

	auto it = stdr::find(edge_vec_, in_dist);
	if(it == edge_vec_.end()){
		return false;
	}
	edge_vec_.erase(it);
	return true;
}
constexpr		bool						dyn_node::try_remove_edge(node_id_t in_dist){
	if constexpr (IS_simple_graph == true){
		if(in_dist == source_id_){
			return false;
		}
	}

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

// dyn_graph implementation start here
namespace SG{

namespace	stdr	= std::ranges;
namespace	stdrv	= std::ranges::views;

constexpr		std::size_t/*O( 1 )*/		dyn_graph::node_size() const{
	return node_table_.size();
}
constexpr		std::size_t/*O( |E| )*/		dyn_graph::edge_size() const{
	std::size_t num_edge{};
	for(const auto& [key, node] : node_table_){
		num_edge += node.edge_vec_.size();
	}
	return num_edge;
}

constexpr		bool						dyn_graph::node_contains(node_id_t in_id) const{
	return node_table_.contains(in_id);
}
constexpr		bool						dyn_graph::edge_contains(alone_edge in_edge) const{
	if constexpr (IS_simple_graph == true) {
		if(in_edge.source == in_edge.dist){
			return false;
		}
	}

	const auto iter = find_node(in_edge.source);
	if(iter != node_table_.end()){
		return iter->second.edge_contains(in_edge.dist);
	}
	return false;
}
constexpr		bool						dyn_graph::edge_contains(node_id_t source, node_id_t dist) const{
	return edge_contains(alone_edge{source, dist});
}

constexpr		std::span<const node_id_t>	dyn_graph::node_edges_span(node_id_t in_id) const{
	auto iter = find_node(in_id);
	if(iter != node_table_.end()){
		return iter->second.edges_span();
	}
	return {};
}

constexpr		dyn_graph::iter_t			dyn_graph::find_node(node_id_t in_id){
	return node_table_.find(in_id);
}
constexpr		dyn_graph::const_iter_t		dyn_graph::find_node(node_id_t in_id) const{
	return node_table_.find(in_id);
}

constexpr		bool						dyn_graph::insert_edge(alone_edge in_edge){
	if constexpr (IS_simple_graph == true) {
		if(in_edge.source == in_edge.dist){
			return false;
		}
	}

	auto [iter, is_new_node] = node_table_.try_emplace(in_edge.source, in_edge.source);
	if(iter->second.insert_edge(in_edge.dist)){
		node_table_.try_emplace(in_edge.dist, in_edge.dist);
		return true;
	}
	return false;
}
constexpr		bool						dyn_graph::insert_edge(node_id_t source, node_id_t dist){
	return insert_edge(alone_edge{source, dist});
}
constexpr		bool						dyn_graph::remove_edge(alone_edge in_edge){
	if constexpr (IS_simple_graph == true) {
		if(in_edge.source == in_edge.dist){
			return false;
		}
	}

	auto iter = find_node(in_edge.source);
	if(iter != node_table_.end()){
		return iter->second.remove_edge(in_edge.dist);
	}
	return false;
}


constexpr		std::size_t					dyn_graph::insert_range(
	input_range_convertible<alone_edge> auto&& in_range
){
	std::size_t failed_count{};

	for(alone_edge ae : in_range){
		if(insert_edge(ae) == false){
			failed_count++;
		}
	}

	return failed_count;
}
constexpr		std::size_t					dyn_graph::remove_range(
	input_range_convertible<alone_edge> auto&& in_range
){
	std::size_t failed_count{};

	for(alone_edge ae : in_range){
		if(remove_edge(ae) == false){
			failed_count++;
		}
	}

	return failed_count;
}

constexpr		std::vector<alone_edge>		dyn_graph::checking_edge_integrity(std::size_t reserve_size) const{
	if (node_table_.empty()) {
        return {};
    }
	
	std::vector<alone_edge> invalid_vec;
	if(reserve_size > 0){
		invalid_vec.reserve(reserve_size);		
	}

	for(const auto& [id, node] : node_table_){
		const node_id_t source = node.source_id_;
		for(const auto dist : node.edges_span()){
			if(node_contains(dist) == false){
				invalid_vec.emplace_back(source, dist);
			}
		}
	}

	return invalid_vec;
}

constexpr		bool						dyn_graph::need_packing_node_id() const{
	if (node_table_.empty()) {
        return false;
    }

	auto iter = stdr::max_element(
		node_table_,
		std::less{},
		&node_table_t::value_type::first
	);

	if(iter->first == (node_table_.size() - 1)){
		return false;
	}
	return true;
}
constexpr		std::vector<node_id_t>		dyn_graph::need_packing_node_id_vec() const{
	if (node_table_.empty()) {
        return {};
    }

    std::vector<node_id_t> id_vec;
    id_vec.reserve(node_table_.size());

    for(const auto& [key, value] : node_table_){
        id_vec.emplace_back(key);
    }

    stdr::sort(id_vec);

	return id_vec;
}

constexpr		dyn_graph::const_id_map_t	dyn_graph::packing_node_id_map() {

	auto id_vec = need_packing_node_id_vec();
	// if already packed
	if (id_vec.back() == id_vec.size() - 1) {
        return {};
    }

    // Build map: Old ID -> New ID
    std::unordered_map<node_id_t, node_id_t> id_map;
    id_map.reserve(node_table_.size());

    node_id_t index_id = 0;
    for(auto node_id : id_vec){
        id_map.try_emplace(node_id, index_id); // Fixed: Old -> New
        index_id++;
    }

    auto packing_dyn_node = [&](node_id_t packed_id, dyn_node& node){
        node.source_id_ = packed_id;
        for(auto& old_id : node.edges_span()){
            auto iter = id_map.find(old_id);
            if (iter != id_map.end()) {
                old_id = iter->second; // Correctly grabs the New ID
            }
        }
    };

    index_id = 0;
    for(auto node_id : id_vec){
        if(index_id == node_id){
            packing_dyn_node(index_id, node_table_.at(node_id));
        }
        else{
            auto table_node = node_table_.extract(node_id);
            table_node.key() = index_id;
            packing_dyn_node(index_id, table_node.mapped());
            node_table_.insert(std::move(table_node));
        }
        index_id++;
    }

    return id_map;
}

constexpr std::vector<node_id_t> dyn_graph::packing_node_id_vec() {
    
	auto id_vec = need_packing_node_id_vec();

    // Efficient packed check
    if (id_vec.back() == id_vec.size() - 1) {
        return {};
    }

    auto packing_dyn_node = [&](node_id_t packed_id, dyn_node& node){
        node.source_id_ = packed_id;
        for(auto& old_id : node.edges_span()){
            auto iter = stdr::lower_bound(id_vec, old_id);
            if (iter != id_vec.end() && *iter == old_id) {
                // Fixed: Correct positive distance calculation
                old_id = static_cast<node_id_t>(std::distance(id_vec.begin(), iter));
            }
        }
    };

    node_id_t index_id = 0;
    for(auto node_id : id_vec){
        if(index_id == node_id){
            packing_dyn_node(index_id, node_table_.at(node_id));
        }
        else{
            auto table_node = node_table_.extract(node_id);
            table_node.key() = index_id;
            packing_dyn_node(index_id, table_node.mapped());
            node_table_.insert(std::move(table_node));
        }
        index_id++;
    }

    return id_vec;
}

}

// std extra formatter and hash here
template <>
struct std::formatter<SG::alone_edge> : std::formatter<std::string>{

	auto format(const SG::alone_edge& e, std::format_context& ctx) const {
        // std::format_to writes directly to the output buffer
        return std::format_to(ctx.out(), "({} -> {}) ", e.source, e.dist);
    }

};