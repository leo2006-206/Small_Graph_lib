module;

export module Small_Graph:core;

import std;

export namespace SG{
	using node_id_t = std::uint32_t;

	using csr_node_t= node_id_t;
	using csr_edge_t= std::uint64_t;

	constexpr bool IS_simple_graph{true};

	template<typename range_t, typename value_t>
	concept input_range_convertible = requires (){
		requires std::ranges::input_range<range_t>;
		requires std::is_convertible_v<std::ranges::range_reference_t<range_t>, value_t>;
	};

	struct	alone_edge{
	node_id_t	source;
	node_id_t	dist;

	explicit constexpr alone_edge(node_id_t in_source, node_id_t in_dist)
	: source(in_source), dist(in_dist){}

	constexpr bool operator==(const alone_edge&) const = default;
};
};