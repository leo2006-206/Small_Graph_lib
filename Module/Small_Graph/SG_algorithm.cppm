module;

export module Small_Graph:algorithm;

import std;
import :core;
import :utility;

// 1. Check if the compiler even supports 128-bit integers 
#if defined(__SIZEOF_INT128__)

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
   
		using uint128_t = unsigned __int128;
    
	#pragma GCC diagnostic pop

#else
    #error "This code requires a compiler with 128-bit integer support."
#endif

export namespace SG{

template<typename graph_t>
concept is_node_edges_range_graph = requires(const graph_t& g,SG::node_id_t id) {
	{ g.node_contains(id) }	-> std::same_as<bool>;
	{ g.node_size() }		-> std::same_as<std::size_t>;
	{ g.edge_size() }		-> std::same_as<std::size_t>;

    { g.node_edges_range(id) }	-> std::ranges::sized_range;
    requires std::same_as<
        std::ranges::range_value_t<decltype(g.node_edges_range(id))>, 
        SG::node_id_t
    >;

	requires std::unsigned_integral<SG::node_id_t>;
};


struct mean_std{
	std::uint64_t	count  = 0;
	std::uint64_t	sum    = 0;
	uint128_t		sum_sq = 0;

	constexpr	void			add(std::uint64_t x);
	constexpr	void			reset();
	constexpr	std::uint64_t	get_mean() const;
	constexpr	std::uint64_t	get_var() const;
	constexpr	std::uint64_t	get_std() const;
};

struct mem_distance{
	SG::node_id_t	prev_source{0};
	std::intptr_t	prev_address{0};
	mean_std		ms{};

	[[gnu::always_inline]]
	constexpr inline
	void operator()(SG::alone_edge e, std::intptr_t p) noexcept{
		if(prev_address == 0)[[unlikely]]{
			// if first time
			prev_source = e.source;
		}
		if(e.source != prev_source)[[likely]]{
			auto diff = static_cast<std::uint64_t>(std::abs(prev_address - p));
			ms.add(diff);
			prev_source = e.source;
		}
		prev_address = p;
	}
};

struct visited_bitvec{
	std::vector<std::uint64_t> bit_vec_;

	explicit			visited_bitvec() = delete;
	explicit constexpr	visited_bitvec(std::size_t num_node){
		bit_vec_.resize(num_node, 0);
	}

	constexpr	bool	is_visited(SG::node_id_t node_index) const;
	constexpr	bool	set_visited(SG::node_id_t node_index);
	constexpr	bool	fetch_set_visited(SG::node_id_t node_index);
};

constexpr void dfs_loop(
	const is_node_edges_range_graph	auto&	graph,
	std::invocable<SG::alone_edge>	auto&&	discover_edge,
	SG::node_id_t							start_id
);

constexpr void dfs_loop_all(
	const is_node_edges_range_graph	auto&	graph,
	std::invocable<SG::alone_edge>	auto&&	discover_edge,
	SG::node_id_t							start_id
);

constexpr SG::mem_distance dfs_loop_mem_dis(
	const is_node_edges_range_graph	auto&	graph,
	SG::node_id_t							start_id
);

}


// struct impl
namespace SG {

constexpr void					mean_std::add(std::uint64_t x){
	count++;
	sum += x;

	uint128_t x128 = x;
	sum_sq += x128 * x128; 
}
constexpr void					mean_std::reset(){
	count = 0;
	sum = 0;
	sum_sq = 0;
}

constexpr std::uint64_t			mean_std::get_mean() const {
	if (count == 0) return 0;
	return sum / count;
}

constexpr std::uint64_t			mean_std::get_var() const{
	if (count == 0) return 0;
	
	double d_count = static_cast<double>(count);
	double mean = static_cast<double>(sum) / d_count;
	double mean_of_squares = static_cast<double>(sum_sq) / d_count;
	double variance = mean_of_squares - (mean * mean);
	return static_cast<std::uint64_t>(variance);
}

constexpr std::uint64_t			mean_std::get_std() const {
	if (count == 0) return 0;
	
	double d_count = static_cast<double>(count);
	double mean = static_cast<double>(sum) / d_count;
	double mean_of_squares = static_cast<double>(sum_sq) / d_count;
	double variance = mean_of_squares - (mean * mean);
	if (variance < 0.0){
		variance = 0.0;
	} 
	return static_cast<std::uint64_t>(std::sqrt(variance));
}

constexpr	bool	visited_bitvec::is_visited(SG::node_id_t node_index) const{
	return bit_vec_[node_index >> 6 /*2^6 == 64*/] & (1 << (node_index & (64 - 1)));
}
constexpr	bool	visited_bitvec::set_visited(SG::node_id_t node_index){
	return bit_vec_[node_index >> 6 /*2^6 == 64*/] |= (1 << (node_index & (64 - 1)));
}
constexpr	bool	visited_bitvec::fetch_set_visited(SG::node_id_t node_index){
	auto& bit_pos = bit_vec_[node_index >> 6 /*2^6 == 64*/];
	const std::uint64_t bitmask = (1 << (node_index & (64 - 1)));
	bool result = bit_pos & bitmask;
	bit_pos |= bitmask;
	return result;
}

}


// function impl
namespace SG {

// __attribute__((noinline))
constexpr void dfs_loop(
	const is_node_edges_range_graph	auto&	graph,
	std::invocable<SG::alone_edge>	auto&&	discover_edge,
	SG::node_id_t							start_id
){	
	if(!graph.node_contains(start_id)){
		return;
	}

	constexpr auto cal_offset = [](node_id_t id) -> std::uint64_t{
		return 1 << (id & 63);
	};

	std::vector<node_id_t> stack;
	std::vector<std::uint64_t> visit_vec;
	visit_vec.resize((graph.node_size() >> 6) + 1, 0);
	stack.reserve(graph.node_size() >> 1);

	stack.emplace_back(start_id);
	visit_vec[start_id >> 6] |= cal_offset(start_id);

	while(!stack.empty()){
		const auto current_node = stack.back();
		stack.pop_back();

		for(const auto current_edge : graph.node_edges_range(current_node)){

			discover_edge(alone_edge{current_node, current_edge});

			auto offset = cal_offset(current_edge);
			auto& bit_pos = visit_vec[current_edge >> 6];

			if((bit_pos & offset) == false)[[unlikely]]{
				bit_pos |= offset;
				stack.emplace_back(current_edge);
			}
		}
	}
}

constexpr void dfs_loop_all(
	const is_node_edges_range_graph	auto&	graph,
	std::invocable<SG::alone_edge>	auto&&	discover_edge,
	SG::node_id_t							start_id
){
	if(!graph.node_contains(start_id)){
		return;
	}

	constexpr auto cal_offset = [](node_id_t id) -> std::uint64_t{
		return 1 << (id & 63);
	};

	std::vector<node_id_t> stack;
	std::vector<std::uint64_t> visit_vec;
	visit_vec.resize((graph.node_size() >> 6) + 1, 0);
	stack.reserve(graph.node_size() >> 1);

	stack.emplace_back(start_id);
	visit_vec[start_id >> 6] |= cal_offset(start_id);

	while(!stack.empty()){
		const auto current_node = stack.back();
		stack.pop_back();

		for(const auto current_edge : graph.node_edges_range(current_node)){

			discover_edge(alone_edge{current_node, current_edge});

			auto offset = cal_offset(current_edge);
			auto& bit_pos = visit_vec[current_edge >> 6];

			if((bit_pos & offset) == false)[[unlikely]]{
				bit_pos |= offset;
				stack.emplace_back(current_edge);
			}
		}
	}
}

constexpr SG::mem_distance dfs_loop_mem_dis(
	const is_node_edges_range_graph	auto&	graph,
	SG::node_id_t							start_id
){	
	if(!graph.node_contains(start_id)){
		return {};
	}

	constexpr auto cal_offset = [](node_id_t id) -> std::uint64_t{
		return 1 << (id & 63);
	};

	std::vector<node_id_t> stack;
	std::vector<std::uint64_t> visit_vec;
	visit_vec.resize((graph.node_size() >> 6) + 1, 0);
	stack.reserve(graph.node_size() >> 1);

	SG::mem_distance md{};
	stack.emplace_back(start_id);
	visit_vec[start_id >> 6] |= cal_offset(start_id);

	while(!stack.empty()){
		const auto current_node = stack.back();
		stack.pop_back();

		for(const auto& current_edge : graph.node_edges_range(current_node)){

			md(
				alone_edge{current_node, current_edge},
				reinterpret_cast<std::intptr_t>(&current_edge)
			);

			auto offset = cal_offset(current_edge);
			auto& bit_pos = visit_vec[current_edge >> 6];

			if((bit_pos & offset) == false)[[unlikely]]{
				bit_pos |= offset;
				stack.emplace_back(current_edge);
			}
		}
	}

	return md;
}

}