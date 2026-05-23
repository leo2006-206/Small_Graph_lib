module;

export module Graph:function;

import std;

export namespace Graph::function {

enum class function_flow : std::uint8_t{
	function_return,
	// stop execution and return from function
	iteration_continue,
	// continue execution
	iteration_skip,
	// skip current iteration of the loop, aka "continue"
	iteration_break
	// break current loop
};

template<typename node_t>
concept is_node = requires (node_t) {
	requires std::is_trivially_copyable_v<node_t>;
	requires std::is_unsigned_v<node_t>;
};

template<typename weight_t>
concept is_weight = requires (weight_t) {
	requires std::is_floating_point_v<weight_t> || std::is_integral_v<weight_t>;
};

template<typename node_t, typename edge_t>
concept is_edge = requires(node_t, edge_t e){
	requires is_node<node_t>;
	requires std::is_trivially_copyable_v<edge_t>;
	e._dest_id;
	e._weight;

	requires std::same_as<node_t, decltype(e._dest_id)>;
	requires is_weight<decltype(e._weight)>;
};

template<typename function, typename node_t>
concept node_function = requires(function){
	requires is_node<node_t>;
	requires std::invocable<function, const node_t>;
	requires std::same_as<std::invoke_result_t<function, const node_t>, function_flow>;
};

template<typename function, typename node_t, typename edge_t>
concept edge_function = requires(function){
	requires is_node<node_t>;
	requires is_edge<node_t, edge_t>;
	requires std::invocable<function, const node_t, const edge_t>;
	requires std::same_as<std::invoke_result_t<function, const node_t, const edge_t>, function_flow>;
};

template<typename function, typename node_t, typename weight>
concept node_cost_function = requires(function){
	requires is_node<node_t>;
	requires is_weight<weight>;
	requires std::invocable<function, const node_t, const weight>;
	requires std::same_as<std::invoke_result_t<function, const node_t, const weight>, function_flow>;
};

template<typename function, typename node_t, typename edge_t>
concept cost_function = requires(function, edge_t e){
	requires is_node<node_t>;
	requires is_edge<node_t, edge_t>;
	requires std::invocable<function, const node_t, const edge_t>;
	requires is_weight<std::invoke_result_t<function, const node_t, const edge_t>>;
};


struct unused_node_func{
	template <typename node_t>
	requires is_node<node_t>
	constexpr auto operator()([[maybe_unused]] const node_t) const{
		return function_flow::iteration_continue;
	}
};
struct unused_edge_func{
	template <typename node_t, typename edge_t>
    requires is_node<node_t> && is_edge<node_t, edge_t>
	constexpr auto operator()([[maybe_unused]] const node_t, [[maybe_unused]] const edge_t) const{
		return function_flow::iteration_continue;
	}
};

struct unused_node_cost_func{
	template<typename node_t, typename cost_t>
	requires is_node<node_t> && is_weight<cost_t>
	constexpr auto operator()([[maybe_unused]] const node_t, [[maybe_unused]] const cost_t) const{
		return function_flow::iteration_continue;
	}
};

struct default_cost_func {
    template <typename node_t, typename edge_t>
    requires is_node<node_t> && is_edge<node_t, edge_t>
    constexpr auto operator()([[maybe_unused]] const node_t, const edge_t e) const{
        return e._weight;
    }
};

}