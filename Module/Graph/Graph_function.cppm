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

	requires std::same_as<node_t, decltype(e._dest_id)>;
};

template<typename node_t, typename edge_t>
concept is_weighted_edge = requires(node_t, edge_t e){
	requires is_node<node_t>;
	requires std::is_trivially_copyable_v<edge_t>;
	e._dest_id;
	e._weight;

	requires std::same_as<node_t, decltype(e._dest_id)>;
	requires is_weight<decltype(e._weight)>;
};

template<typename visitor, typename node_t, typename edge_t>
concept is_non_cost_graph_visitor = requires(const visitor v, const node_t n, const edge_t e){
	requires is_node<node_t>;
	requires is_edge<node_t, edge_t>;
	requires std::is_empty_v<visitor>;
	
	{v.find_node(n)}	-> std::same_as<function_flow>;
	{v.find_edge(n, e)}	-> std::same_as<function_flow>;
	{v.end_node(n)}		-> std::same_as<function_flow>;
};

template<typename visitor, typename node_t, typename edge_t>
concept is_cost_graph_visitor = requires(
	const visitor v,
	const node_t n,
	const edge_t e,
	const typename visitor::cost_type w
){
	requires is_node<node_t>;
	requires is_weighted_edge<node_t, edge_t>;
	requires is_weight<typename visitor::cost_type>;
	requires std::is_empty_v<visitor>;
	
	{v.find_node(n, w)}		-> std::same_as<function_flow>;
	{v.find_edge(n, w, e)}	-> std::same_as<function_flow>;
	{v.cal_cost(n, w, e)}	-> std::same_as<typename visitor::cost_type>;
	{v.end_node(n, w)}		-> std::same_as<function_flow>;
};

struct default_non_cost_visitor {
    constexpr function_flow find_node(const auto/*node*/) const{
        return function_flow::iteration_continue;
    }
    constexpr function_flow find_edge(const auto/*node*/, const auto/*edge*/) const{
        return function_flow::iteration_continue;
    }
    constexpr function_flow end_node(const auto/*node*/) const{
        return function_flow::iteration_continue;
    }
};

template <typename cost_t>
requires is_weight<cost_t>
struct default_cost_visitor {
    using cost_type = cost_t;

    constexpr function_flow find_node(const auto/*node*/, const cost_t) const{
        return function_flow::iteration_continue;
    }
    constexpr function_flow find_edge(const auto/*node*/, const cost_t, const auto/*edge*/) const{
        return function_flow::iteration_continue;
    }
    constexpr cost_t cal_cost(const auto/*node*/, const cost_t curr_total_cost, const auto new_edge) const{
        return curr_total_cost + new_edge._weight; 
    }
    constexpr function_flow end_node(const auto/*node*/, const cost_t) const{
        return function_flow::iteration_continue;
    }
};

}

namespace stand_alone_function_api{
	// !!!DO NOT REMOVE THIS PART!!!
	// !!!DO NOT REMOVE THIS PART!!!
	// !!!DO NOT REMOVE THIS PART!!!

// template<typename function, typename node_t>
// concept node_function = requires(function){
// 	requires is_node<node_t>;
// 	requires std::invocable<function, const node_t>;
// 	requires std::same_as<std::invoke_result_t<function, const node_t>, function_flow>;
// };

// template<typename function, typename node_t, typename edge_t>
// concept edge_function = requires(function){
// 	requires is_node<node_t>;
// 	requires is_edge<node_t, edge_t>;
// 	requires std::invocable<function, const node_t, const edge_t>;
// 	requires std::same_as<std::invoke_result_t<function, const node_t, const edge_t>, function_flow>;
// };

// template<typename function, typename node_t, typename edge_t>
// concept cost_function = requires(function, edge_t e){
// 	requires is_node<node_t>;
// 	requires is_edge<node_t, edge_t>;
// 	requires std::invocable<function, const node_t, const edge_t>;
// 	requires is_weight<std::invoke_result_t<function, const node_t, const edge_t>>;
// };

// template<typename function, typename node_t, typename weight_t>
// concept node_cost_function = requires(function){
// 	requires is_node<node_t>;
// 	requires is_weight<weight_t>;
// 	requires std::invocable<function, const node_t, const weight_t>;
// 	requires std::same_as<std::invoke_result_t<function, const node_t, const weight_t>, function_flow>;
// };

// template<typename function, typename node_t, typename edge_t, typename weight_t>
// concept edge_cost_function = requires(function){
// 	requires is_node<node_t>;
// 	requires is_edge<node_t, edge_t>;
// 	requires is_weight<weight_t>;
// 	requires std::invocable<function, const node_t, const edge_t, const weight_t>;
// 	requires std::same_as<std::invoke_result_t<function, const node_t, const edge_t, const weight_t>, function_flow>;
// };

// struct unused_node_func{
// 	template <typename node_t>
// 	requires is_node<node_t>
// 	constexpr auto operator()([[maybe_unused]] const node_t) const{
// 		return function_flow::iteration_continue;
// 	}
// };
// struct unused_edge_func{
// 	template <typename node_t, typename edge_t>
//     requires is_node<node_t> && is_edge<node_t, edge_t>
// 	constexpr auto operator()([[maybe_unused]] const node_t, [[maybe_unused]] const edge_t) const{
// 		return function_flow::iteration_continue;
// 	}
// };

// struct unused_node_cost_func{
// 	template<typename node_t, typename cost_t>
// 	requires is_node<node_t> && is_weight<cost_t>
// 	constexpr auto operator()([[maybe_unused]] const node_t, [[maybe_unused]] const cost_t) const{
// 		return function_flow::iteration_continue;
// 	}
// };

// struct unused_edge_cost_func{
// 	template<typename node_t, typename edge_t, typename cost_t>
// 	requires is_node<node_t> && is_edge<node_t, edge_t> && is_weight<cost_t>
// 	constexpr auto operator()([[maybe_unused]] const node_t, [[maybe_unused]] const cost_t) const{
// 		return function_flow::iteration_continue;
// 	}
// };

// struct default_cost_func {
//     template <typename node_t, typename edge_t>
//     requires is_node<node_t> && is_edge<node_t, edge_t>
//     constexpr auto operator()([[maybe_unused]] const node_t, const edge_t e) const{
//         return e._weight;
//     }
// };
}