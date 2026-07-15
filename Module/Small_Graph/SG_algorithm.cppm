module;

#include <cassert>

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

template<typename visited_t>
concept is_visited = requires (visited_t& v, SG::node_id_t id){
	{ v.is_visited(id) }	-> std::same_as<bool>;
	{ v.set_visited(id) }	-> std::same_as<bool>;
	{ v.fetch_set_visited(id)}	-> std::same_as<bool>;
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
		bit_vec_.resize((num_node + 63) / 64, 0);
	}

	constexpr	bool	is_visited(SG::node_id_t node_index) const;
	constexpr	bool	set_visited(SG::node_id_t node_index);
	constexpr	bool	fetch_set_visited(SG::node_id_t node_index);
};

struct next_visited_bitvec{
	visited_bitvec	visit_vec_;
	std::size_t		word_index;

	explicit			next_visited_bitvec() = delete;
	explicit constexpr	next_visited_bitvec(std::size_t num_node)
	: visit_vec_(num_node), word_index(0){}

	constexpr	std::optional<SG::node_id_t>	next_unvisited();
	constexpr	bool	is_visited(SG::node_id_t node_index) const{
		return visit_vec_.is_visited(node_index);
	}
	constexpr	bool	set_visited(SG::node_id_t node_index){
		return visit_vec_.set_visited(node_index);
	}
	constexpr	bool	fetch_set_visited(SG::node_id_t node_index){
		return visit_vec_.fetch_set_visited(node_index);
	}
};

constexpr void dfs_core(
	const is_node_edges_range_graph	auto&	graph,
	std::vector<SG::node_id_t>&				stack,	
	is_visited						auto&	visit_vec,
	std::invocable<SG::alone_edge>	auto&&	discover_edge,
	SG::node_id_t							start_id	
);

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

constexpr SG::mem_distance bfs_loop_mem_dis(
	const is_node_edges_range_graph	auto&	graph,
	SG::node_id_t							start_id
);

constexpr
std::tuple<
	std::size_t,
	SG::mem_distance,//span_i
	SG::mem_distance//span_j
> count_triangle_mem_dis(
	const is_node_edges_range_graph auto&	graph
);

constexpr std::vector<double> page_rank(
	const is_node_edges_range_graph	auto&	graph,
	std::uint32_t							max_iteration,
	double									damping_factor = 0.85,
	double									threshold = 1e-6
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
	return bit_vec_[node_index >> 6 /*2^6 == 64*/] & (1ULL << (node_index & (64 - 1)));
}
constexpr	bool	visited_bitvec::set_visited(SG::node_id_t node_index){
	return bit_vec_[node_index >> 6 /*2^6 == 64*/] |= (1ULL << (node_index & (64 - 1)));
}
constexpr	bool	visited_bitvec::fetch_set_visited(SG::node_id_t node_index){
	auto& bit_pos = bit_vec_[node_index >> 6 /*2^6 == 64*/];
	const std::uint64_t bitmask = (1ULL << (node_index & (64 - 1)));
	bool result = bit_pos & bitmask;
	bit_pos |= bitmask;
	return result;
}

constexpr	std::optional<SG::node_id_t>	next_visited_bitvec::next_unvisited(){
	while(word_index < visit_vec_.bit_vec_.size()){
		const auto& word = visit_vec_.bit_vec_[word_index];
		if( word != (~0ULL) ){
			int bit_pos = std::countr_one(word);
			return static_cast<node_id_t>((word_index * 64) + std::size_t(bit_pos));
		}
		word_index++;
	}

	return std::nullopt;
}

}

template <>
struct std::formatter<SG::mem_distance> : std::formatter<std::string>{
	auto format(const SG::mem_distance& md, std::format_context& ctx) const{
        // std::format_to writes directly to the output buffer
        return std::format_to(
			ctx.out(), 
			"mem_dis\n\tcount\t:\t{}\n\tmean\t:\t{}\n\tstd\t:\t{}",
			md.ms.count, md.ms.get_mean(), md.ms.get_std()
		);
    }
};


// function impl
namespace SG {

constexpr void dfs_core(
	const is_node_edges_range_graph	auto&	graph,
	std::vector<SG::node_id_t>&				stack,	
	is_visited						auto&	visit_vec,
	std::invocable<SG::alone_edge>	auto&&	discover_edge,
	SG::node_id_t							start_id	
){
	stack.emplace_back(start_id);
	visit_vec.set_visited(start_id);

	while(!stack.empty()){
		const auto current_node = stack.back();
		stack.pop_back();

		for(const auto current_edge : graph.node_edges_range(current_node)){
			discover_edge(alone_edge{current_node, current_edge});
			if(!visit_vec.fetch_set_visited(current_edge))[[unlikely]]{
				stack.emplace_back(current_edge);
			}
		}
	}
}

// __attribute__((noinline))
constexpr void dfs_loop(
	const is_node_edges_range_graph	auto&	graph,
	std::invocable<SG::alone_edge>	auto&&	discover_edge,
	SG::node_id_t							start_id
){	
	if(!graph.node_contains(start_id)){
		return;
	}

	std::vector<node_id_t> stack;
	visited_bitvec visit_vec(graph.node_size());
	stack.reserve(graph.node_size() >> 1);

	dfs_core(
		graph,
		stack,
		visit_vec,
		discover_edge,
		start_id
	);
}

constexpr void dfs_loop_all(
	const is_node_edges_range_graph	auto&	graph,
	std::invocable<SG::alone_edge>	auto&&	discover_edge,
	SG::node_id_t							start_id
){
	if(!graph.node_contains(start_id)){
		return;
	}

	std::vector<node_id_t> stack;
	next_visited_bitvec visit_vec(graph.node_size());
	stack.reserve(graph.node_size() >> 1);

	// std::size_t num_sub_g{};

	for(
		std::optional<node_id_t> opt_start{start_id};
		opt_start.has_value();
		opt_start = visit_vec.next_unvisited()
	){
		auto& this_start = *opt_start;

		dfs_core(
			graph,
			stack,
			visit_vec,
			discover_edge,
			this_start
		);

		// num_sub_g++;
	}

	// std::println("\nfinal number sub graph = {}", num_sub_g);
}

constexpr SG::mem_distance dfs_loop_mem_dis(
	const is_node_edges_range_graph	auto&	graph,
	SG::node_id_t							start_id
){	
	if(!graph.node_contains(start_id)){
		return {};
	}

	std::vector<node_id_t> stack;
	visited_bitvec visit_vec(graph.node_size());
	stack.reserve(graph.node_size() >> 1);

	mem_distance md{};
	stack.emplace_back(start_id);
	visit_vec.set_visited(start_id);

	while(!stack.empty()){
		const auto current_node = stack.back();
		stack.pop_back();

		for(const auto& current_edge : graph.node_edges_range(current_node)){
			md(
				alone_edge{current_node, current_edge},
				reinterpret_cast<std::intptr_t>(&current_edge)
			);
			if(!visit_vec.fetch_set_visited(current_edge))[[unlikely]]{
				stack.emplace_back(current_edge);
			}
		}
	}

	return md;
}

constexpr SG::mem_distance bfs_loop_mem_dis(
	const is_node_edges_range_graph	auto&	graph,
	SG::node_id_t							start_id
){	
	if(!graph.node_contains(start_id)){
		return {};
	}

	std::queue<node_id_t, std::deque<node_id_t>> queue;
	visited_bitvec visit_vec(graph.node_size());
	// queue.reserve(graph.node_size() >> 1);

	mem_distance md{};
	queue.push(start_id);
	visit_vec.set_visited(start_id);

	while(!queue.empty()){
		const auto current_node = queue.front();
		queue.pop();

		for(const auto& current_edge : graph.node_edges_range(current_node)){
			md(
				alone_edge{current_node, current_edge},
				reinterpret_cast<std::intptr_t>(&current_edge)
			);
			if(!visit_vec.fetch_set_visited(current_edge))[[unlikely]]{
				queue.push(current_edge);
			}
		}
	}

	return md;
}

constexpr
std::tuple<
	std::size_t,
	SG::mem_distance,//span_i
	SG::mem_distance//span_j
> count_triangle_mem_dis(
	const is_node_edges_range_graph auto&	graph
){
	std::size_t num_tri{};
	SG::mem_distance md_i{};
	SG::mem_distance md_j{};
	
	for(node_id_t node_i = 0; node_i < graph.node_size(); ++node_i){
		for(const node_id_t& node_j : graph.node_edges_range(node_i)){
			if(node_j <= node_i){
				continue;
			}

			std::span	span_i = graph.node_edges_range(node_i);
			std::span	span_j = graph.node_edges_range(node_j);
			std::size_t	idx_i{};
			std::size_t idx_j{};

			while(idx_i < span_i.size() && idx_j < span_j.size()){
				const node_id_t& k_i = span_i[idx_i]; //md??
				const node_id_t& k_j = span_j[idx_j]; //md??

				//the source node_j is only a marker,
				//that if node_j changed, it will record the differ
				md_i(
					alone_edge{node_j, k_i},
					reinterpret_cast<std::intptr_t>(&k_i)
				);
				md_j(
					alone_edge{node_j, k_j},
					reinterpret_cast<std::intptr_t>(&k_j)
				);

				if(k_i <= node_j){
					idx_i++;
					continue;
				}
				if(k_j <= node_j){
					idx_j++;
					continue;
				}

				if(k_i == k_j){
					num_tri++;
					idx_i++;
					idx_j++;
				}
				else if(k_i < k_j){
					idx_i++;
				}
				else{
					idx_j++;
				}
			}

		}
	}

	return {num_tri, md_i, md_j};
}

constexpr std::vector<double> page_rank(
	const is_node_edges_range_graph	auto&	graph,
	std::uint32_t							max_iteration,
	double									damping_factor,
	double									threshold
){
	const auto num_node = graph.node_size();
	const auto num_node_f = static_cast<double>(num_node);

	std::vector<double> rank;
	std::vector<double> next_rank;
	rank.resize(num_node, 1.0 / num_node_f);
	next_rank.resize(num_node, 0.0);

	auto part1 = [&]() -> double{
		double dangling_sum{};

		for(node_id_t u{}; u < num_node; ++u){
			std::span			outgoing = graph.node_edges_range(u);
			const std::size_t	out_degree = outgoing.size();

			if(out_degree > 0){
				double distribute = rank[u] / static_cast<double>(out_degree);
				for(const auto& v : outgoing){

					if(graph.node_contains(v) == false){
						std::cout << "not exist v = "<<v;
					}

					next_rank[v] += distribute;
				}
			}
			else{
				dangling_sum += rank[u];
			}
		}

		return dangling_sum;
	};

	auto part2 = [&](double dangling_sum) -> double{
		double base_rank = (1.0 - damping_factor) / num_node_f;
        double dangling_distribution = (damping_factor * dangling_sum) / num_node_f;
        double total_diff{};

		for(node_id_t u{}; u < num_node; ++u){
			next_rank[u] = base_rank + dangling_distribution + (damping_factor * next_rank[u]);
			total_diff += std::abs(next_rank[u] - rank[u]);

			rank[u] = next_rank[u];
			next_rank[u] = 0.0;
		}

		return total_diff;
	};

	for(auto _ : std::views::iota(0U, max_iteration)){
		double sum = part1();
		double diff = part2(sum);

		if(diff < threshold){
			break;
		}
	}

	return rank;
}

}