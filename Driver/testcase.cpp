import std;
import Graph;

using Graph = Small_Graph::dyn_dir_graph;
using NodeId = Small_Graph::node_id_t;
using Edge = Small_Graph::alone_edge;
using DynNode = Small_Graph::dyn_node;

static Graph load_graph_from_file(const std::string& filename) {
	Graph dg;
	std::vector<Edge> al_vec;
	al_vec.reserve(420045);

	std::ifstream file(filename);
	std::string line;

	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '%' || line[0] == '#') {
			continue;
		}

		std::replace(line.begin(), line.end(), ',', ' ');
		std::istringstream iss(line);

		NodeId source;
		NodeId dest;
		if (iss >> source >> dest) {
			al_vec.emplace_back(source, dest);
		}
	}

	dg.insert_alone_edges(al_vec, al_vec.capacity());
	return dg;
}

static void reset_graph(Graph& graph) {
	graph = Graph{};
}

static void build_two_node_one_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_edge_with_node(1, 2);
}

static void build_small_graph(Graph& graph) {
	reset_graph(graph);
	graph.insert_edge_with_node(1, 2);
	graph.insert_edge_with_node(1, 3);
	graph.insert_edge_with_node(2, 3);
}

static bool find_any_node_id(Graph& graph, NodeId& out_id) {
	for (auto it = graph.begin(); it != graph.end(); ++it) {
		out_id = it->first;
		return true;
	}
	return false;
}

static bool find_any_edge(Graph& graph, NodeId& out_source, NodeId& out_dest) {
	for (auto it = graph.begin(); it != graph.end(); ++it) {
		const auto span = it->second.edges_span();
		if (!span.empty()) {
			out_source = it->first;
			out_dest = span.front();
			return true;
		}
	}
	return false;
}

// dyn_node tests

static bool test_dyn_node_ctor_empty(Graph&) {
	DynNode node{1};
	return node._id == 1 && node.edges_span().empty();
}

static bool test_dyn_node_ctor_edge(Graph&) {
	DynNode node{2, 0};
	return node._id == 2 && node.edges_span().empty();
}

static bool test_dyn_node_ctor_normal(Graph&) {
	DynNode node{3, 128};
	return node._id == 3 && node.edges_span().empty();
}

static bool test_dyn_node_operator_eq_empty(Graph&) {
	DynNode a{1};
	DynNode b{1};
	return a == b;
}

static bool test_dyn_node_operator_eq_edge(Graph&) {
	DynNode a{1};
	DynNode b{2};
	return !(a == b);
}

static bool test_dyn_node_operator_eq_normal(Graph&) {
	DynNode a{1};
	DynNode b{1};
	a.insert_edge(2);
	return a == b;
}

static bool test_dyn_node_edge_contains_empty(Graph&) {
	DynNode node{1};
	return !node.edge_contains(2);
}

static bool test_dyn_node_edge_contains_edge(Graph&) {
	DynNode node{1};
	node.insert_edge(2);
	return node.edge_contains(2) && !node.edge_contains(3);
}

static bool test_dyn_node_edge_contains_normal(Graph&) {
	DynNode node{1};
	node.insert_edge(2);
	node.insert_edge(3);
	return node.edge_contains(3);
}

static bool test_dyn_node_edges_span_empty(Graph&) {
	DynNode node{1};
	return node.edges_span().empty();
}

static bool test_dyn_node_edges_span_edge(Graph&) {
	DynNode node{1};
	node.insert_edge(2);
	const auto span = node.edges_span();
	return span.size() == 1 && span[0] == 2;
}

static bool test_dyn_node_edges_span_normal(Graph&) {
	DynNode node{1};
	node.insert_edge(2);
	node.insert_edge(3);
	auto span = node.edges_span();
	span[0] = 9;
	const auto& cnode = node;
	const auto cspan = cnode.edges_span();
	return cspan.size() == 2 && cspan[0] == 9;
}

static bool test_dyn_node_edges_span_const_empty(Graph&) {
	DynNode node{1};
	const DynNode& cnode = node;
	return cnode.edges_span().empty();
}

static bool test_dyn_node_edges_span_const_edge(Graph&) {
	DynNode node{1};
	node.insert_edge(2);
	const DynNode& cnode = node;
	const auto span = cnode.edges_span();
	return span.size() == 1 && span[0] == 2;
}

static bool test_dyn_node_edges_span_const_normal(Graph&) {
	DynNode node{1};
	node.insert_edge(2);
	node.insert_edge(3);
	const DynNode& cnode = node;
	return cnode.edges_span().size() == 2;
}

static bool test_dyn_node_insert_edge_empty(Graph&) {
	DynNode node{1};
	return node.insert_edge(2) && node.edges_span().size() == 1;
}

static bool test_dyn_node_insert_edge_edge(Graph&) {
	DynNode node{1};
	return !node.insert_edge(1) && node.edges_span().empty();
}

static bool test_dyn_node_insert_edge_normal(Graph&) {
	DynNode node{1};
	node.insert_edge(2);
	return !node.insert_edge(2) && node.edges_span().size() == 1;
}

static bool test_dyn_node_insert_edge_ref_empty(Graph&) {
	DynNode node{1};
	std::size_t count = 0;
	const bool inserted = node.insert_edge(2, count);
	return inserted && count == 1;
}

static bool test_dyn_node_insert_edge_ref_edge(Graph&) {
	DynNode node{1};
	std::size_t count = 0;
	const bool inserted = node.insert_edge(1, count);
	return !inserted && count == 0;
}

static bool test_dyn_node_insert_edge_ref_normal(Graph&) {
	DynNode node{1};
	std::size_t count = 0;
	node.insert_edge(2, count);
	const bool inserted = node.insert_edge(2, count);
	return !inserted && count == 1;
}

static bool test_dyn_node_remove_edge_empty(Graph&) {
	DynNode node{1};
	return !node.remove_edge(2);
}

static bool test_dyn_node_remove_edge_edge(Graph&) {
	DynNode node{1};
	node.insert_edge(2);
	if (!node.remove_edge(2)) {
		return false;
	}
	return !node.remove_edge(2) && node.edges_span().empty();
}

static bool test_dyn_node_remove_edge_normal(Graph&) {
	DynNode node{1};
	node.insert_edge(2);
	node.insert_edge(3);
	node.insert_edge(4);
	if (!node.remove_edge(3)) {
		return false;
	}
	return !node.edge_contains(3) && node.edge_contains(2) && node.edge_contains(4);
}

static bool test_dyn_node_sort_edge_empty(Graph&) {
	DynNode node{1};
	node.sort_edge();
	return node.edges_span().empty();
}

static bool test_dyn_node_sort_edge_edge(Graph&) {
	DynNode node{0};
	node.insert_edge(5);
	node.insert_edge(3);
	node.insert_edge(1);
	node.sort_edge();
	const auto span = node.edges_span();
	return span.size() == 3 && span[0] == 5 && span[1] == 3 && span[2] == 1;
}

static bool test_dyn_node_sort_edge_normal(Graph&) {
	DynNode node{1};
	node.insert_edge(2);
	node.insert_edge(5);
	node.insert_edge(3);
	node.sort_edge();
	const auto span = node.edges_span();
	return span.size() == 3 && span[0] == 5 && span[1] == 3 && span[2] == 2;
}

// dyn_dir_graph tests

static bool test_dyn_dir_graph_ctor_empty(Graph&) {
	Graph graph;
	return graph.node_size() == 0 && graph.edge_size() == 0;
}

static bool test_dyn_dir_graph_ctor_edge(Graph&) {
	Graph graph(1);
	graph.insert_node(1);
	return graph.node_size() == 1;
}

static bool test_dyn_dir_graph_ctor_normal(Graph&) {
	Graph graph(8);
	graph.insert_edge_with_node(1, 2);
	return graph.node_size() == 2 && graph.edge_size() == 1;
}

static bool test_dyn_dir_graph_node_contains_empty(Graph& graph) {
	reset_graph(graph);
	return !graph.node_contains(1);
}

static bool test_dyn_dir_graph_node_contains_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	return graph.node_contains(1) && !graph.node_contains(2);
}

static bool test_dyn_dir_graph_node_contains_normal(Graph& graph) {
	NodeId id = 0;
	if (!find_any_node_id(graph, id)) {
		return false;
	}
	return graph.node_contains(id);
}

static bool test_dyn_dir_graph_edge_contains_empty(Graph& graph) {
	reset_graph(graph);
	return !graph.edge_contains(1, 2);
}

static bool test_dyn_dir_graph_edge_contains_edge(Graph& graph) {
	build_two_node_one_edge(graph);
	return graph.edge_contains(1, 2) && !graph.edge_contains(2, 1);
}

static bool test_dyn_dir_graph_edge_contains_normal(Graph& graph) {
	NodeId source = 0;
	NodeId dest = 0;
	if (!find_any_edge(graph, source, dest)) {
		return false;
	}
	const NodeId invalid = std::numeric_limits<NodeId>::max();
	return graph.edge_contains(source, dest) && !graph.edge_contains(source, invalid);
}

static bool test_dyn_dir_graph_node_size_empty(Graph& graph) {
	reset_graph(graph);
	return graph.node_size() == 0;
}

static bool test_dyn_dir_graph_node_size_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	return graph.node_size() == 1;
}

static bool test_dyn_dir_graph_node_size_normal(Graph& graph) {
	return graph.node_size() > 0;
}

static bool test_dyn_dir_graph_edge_size_empty(Graph& graph) {
	reset_graph(graph);
	return graph.edge_size() == 0;
}

static bool test_dyn_dir_graph_edge_size_edge(Graph& graph) {
	build_two_node_one_edge(graph);
	return graph.edge_size() == 1;
}

static bool test_dyn_dir_graph_edge_size_normal(Graph& graph) {
	return graph.edge_size() == graph.edge_size_cache();
}

static bool test_dyn_dir_graph_edge_size_cache_empty(Graph& graph) {
	reset_graph(graph);
	return graph.edge_size_cache() == 0 && graph.edge_size() == 0;
}

static bool test_dyn_dir_graph_edge_size_cache_edge(Graph& graph) {
	build_small_graph(graph);
	return graph.edge_size_cache() == 3 && graph.edge_size() == 3;
}

static bool test_dyn_dir_graph_edge_size_cache_normal(Graph& graph) {
	return graph.edge_size_cache() > 0;
}

static bool test_dyn_dir_graph_begin_end_empty(Graph& graph) {
	reset_graph(graph);
	return graph.begin() == graph.end();
}

static bool test_dyn_dir_graph_begin_end_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	return graph.begin() != graph.end() && graph.begin()->first == 1;
}

static bool test_dyn_dir_graph_begin_end_normal(Graph& graph) {
	build_small_graph(graph);
	const auto count = static_cast<std::size_t>(std::distance(graph.begin(), graph.end()));
	return count == graph.node_size();
}

static bool test_dyn_dir_graph_cbegin_cend_empty(Graph& graph) {
	reset_graph(graph);
	return graph.cbegin() == graph.cend();
}

static bool test_dyn_dir_graph_cbegin_cend_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	return graph.cbegin() != graph.cend();
}

static bool test_dyn_dir_graph_cbegin_cend_normal(Graph& graph) {
	build_small_graph(graph);
	const auto count = static_cast<std::size_t>(std::distance(graph.cbegin(), graph.cend()));
	return count == graph.node_size();
}

static bool test_dyn_dir_graph_node_range_empty(Graph& graph) {
	reset_graph(graph);
	const auto range = graph.node_range();
	return std::ranges::distance(range) == 0;
}

static bool test_dyn_dir_graph_node_range_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	const auto range = graph.node_range();
	auto it = range.begin();
	if (it == range.end()) {
		return false;
	}
	return (*it)._id == 1;
}

static bool test_dyn_dir_graph_node_range_normal(Graph& graph) {
	build_small_graph(graph);
	const auto range = graph.node_range();
	return std::ranges::distance(range) == 3;
}

static bool test_dyn_dir_graph_node_range_const_empty(Graph& graph) {
	reset_graph(graph);
	const Graph& cgraph = graph;
	const auto range = cgraph.node_range();
	return std::ranges::distance(range) == 0;
}

static bool test_dyn_dir_graph_node_range_const_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	const Graph& cgraph = graph;
	const auto range = cgraph.node_range();
	auto it = range.begin();
	if (it == range.end()) {
		return false;
	}
	return (*it)._id == 1;
}

static bool test_dyn_dir_graph_node_range_const_normal(Graph& graph) {
	const Graph& cgraph = graph;
	const auto range = cgraph.node_range();
	return std::ranges::distance(range) == static_cast<std::ptrdiff_t>(graph.node_size());
}

static bool test_dyn_dir_graph_node_range_sized_empty(Graph& graph) {
	reset_graph(graph);
	const Graph& cgraph = graph;
	const auto range = cgraph.node_range_sized();
	return std::ranges::size(range) == 0;
}

static bool test_dyn_dir_graph_node_range_sized_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	const Graph& cgraph = graph;
	const auto range = cgraph.node_range_sized();
	return std::ranges::size(range) == 1;
}

static bool test_dyn_dir_graph_node_range_sized_normal(Graph& graph) {
	const Graph& cgraph = graph;
	const auto range = cgraph.node_range_sized();
	return std::ranges::size(range) == graph.node_size();
}

static bool test_dyn_dir_graph_node_edges_span_empty(Graph& graph) {
	reset_graph(graph);
	return graph.node_edges_span(1).empty();
}

static bool test_dyn_dir_graph_node_edges_span_edge(Graph& graph) {
	build_two_node_one_edge(graph);
	const auto span = graph.node_edges_span(1);
	return span.size() == 1 && span[0] == 2;
}

static bool test_dyn_dir_graph_node_edges_span_normal(Graph& graph) {
	build_small_graph(graph);
	auto span = graph.node_edges_span(1);
	if (span.size() < 2) {
		return false;
	}
	span[0] = 9;
	return graph.edge_contains(1, 9);
}

static bool test_dyn_dir_graph_node_edges_span_const_empty(Graph& graph) {
	reset_graph(graph);
	const Graph& cgraph = graph;
	return cgraph.node_edges_span(1).empty();
}

static bool test_dyn_dir_graph_node_edges_span_const_edge(Graph& graph) {
	build_two_node_one_edge(graph);
	const Graph& cgraph = graph;
	const auto span = cgraph.node_edges_span(1);
	return span.size() == 1 && span[0] == 2;
}

static bool test_dyn_dir_graph_node_edges_span_const_normal(Graph& graph) {
	NodeId source = 0;
	NodeId dest = 0;
	if (!find_any_edge(graph, source, dest)) {
		return false;
	}
	const Graph& cgraph = graph;
	return !cgraph.node_edges_span(source).empty();
}

static bool test_dyn_dir_graph_node_edges_range_empty(Graph& graph) {
	reset_graph(graph);
	const auto range = graph.node_edges_range(1);
	return std::ranges::distance(range) == 0;
}

static bool test_dyn_dir_graph_node_edges_range_edge(Graph& graph) {
	build_two_node_one_edge(graph);
	const auto range = graph.node_edges_range(1);
	if (std::ranges::distance(range) != 1) {
		return false;
	}
	return *range.begin() == 2;
}

static bool test_dyn_dir_graph_node_edges_range_normal(Graph& graph) {
	build_small_graph(graph);
	const auto range = graph.node_edges_range(1);
	return std::ranges::distance(range) == 2;
}

static bool test_dyn_dir_graph_node_edges_range_const_empty(Graph& graph) {
	reset_graph(graph);
	const Graph& cgraph = graph;
	const auto range = cgraph.node_edges_range(1);
	return std::ranges::distance(range) == 0;
}

static bool test_dyn_dir_graph_node_edges_range_const_edge(Graph& graph) {
	build_two_node_one_edge(graph);
	const Graph& cgraph = graph;
	const auto range = cgraph.node_edges_range(1);
	if (std::ranges::distance(range) != 1) {
		return false;
	}
	return *range.begin() == 2;
}

static bool test_dyn_dir_graph_node_edges_range_const_normal(Graph& graph) {
	NodeId source = 0;
	NodeId dest = 0;
	if (!find_any_edge(graph, source, dest)) {
		return false;
	}
	const Graph& cgraph = graph;
	const auto range = cgraph.node_edges_range(source);
	return std::ranges::distance(range) > 0;
}

static bool test_dyn_dir_graph_node_edges_range_sized_empty(Graph& graph) {
	reset_graph(graph);
	const Graph& cgraph = graph;
	const auto range = cgraph.node_edges_range_sized(1);
	return std::ranges::size(range) == 0;
}

static bool test_dyn_dir_graph_node_edges_range_sized_edge(Graph& graph) {
	build_two_node_one_edge(graph);
	const Graph& cgraph = graph;
	const auto range = cgraph.node_edges_range_sized(1);
	return std::ranges::size(range) == 1;
}

static bool test_dyn_dir_graph_node_edges_range_sized_normal(Graph& graph) {
	NodeId source = 0;
	NodeId dest = 0;
	if (!find_any_edge(graph, source, dest)) {
		return false;
	}
	const Graph& cgraph = graph;
	const auto range = cgraph.node_edges_range_sized(source);
	return std::ranges::size(range) > 0;
}

static bool test_dyn_dir_graph_access_node_ptr_empty(Graph& graph) {
	reset_graph(graph);
	return graph.access_node_ptr(1) == nullptr;
}

static bool test_dyn_dir_graph_access_node_ptr_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	const auto ptr = graph.access_node_ptr(1);
	return ptr != nullptr && ptr->_id == 1;
}

static bool test_dyn_dir_graph_access_node_ptr_normal(Graph& graph) {
	NodeId id = 0;
	if (!find_any_node_id(graph, id)) {
		return false;
	}
	return graph.access_node_ptr(id) != nullptr;
}

static bool test_dyn_dir_graph_access_node_ptr_const_empty(Graph& graph) {
	reset_graph(graph);
	const Graph& cgraph = graph;
	return cgraph.access_node_ptr(1) == nullptr;
}

static bool test_dyn_dir_graph_access_node_ptr_const_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	const Graph& cgraph = graph;
	const auto ptr = cgraph.access_node_ptr(1);
	return ptr != nullptr && ptr->_id == 1;
}

static bool test_dyn_dir_graph_access_node_ptr_const_normal(Graph& graph) {
	NodeId id = 0;
	if (!find_any_node_id(graph, id)) {
		return false;
	}
	const Graph& cgraph = graph;
	return cgraph.access_node_ptr(id) != nullptr;
}

static bool test_dyn_dir_graph_checking_edge_integrity_empty(Graph& graph) {
	reset_graph(graph);
	const auto errors = graph.checking_edge_integrity();
	return errors.empty();
}

static bool test_dyn_dir_graph_checking_edge_integrity_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	graph.insert_edge(1, 2);
	const auto errors = graph.checking_edge_integrity();
	if (errors.size() != 1) {
		return false;
	}
	return errors[0]._source == 1 && errors[0]._dest == 2;
}

static bool test_dyn_dir_graph_checking_edge_integrity_normal(Graph& graph) {
	const auto errors = graph.checking_edge_integrity();
	return errors.empty();
}

static bool test_dyn_dir_graph_insert_node_empty(Graph& graph) {
	reset_graph(graph);
	return graph.insert_node(1) != nullptr && graph.node_size() == 1;
}

static bool test_dyn_dir_graph_insert_node_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	return graph.insert_node(1) == nullptr && graph.node_size() == 1;
}

static bool test_dyn_dir_graph_insert_node_normal(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	graph.insert_node(2);
	graph.insert_node(3);
	return graph.node_size() == 3;
}

static bool test_dyn_dir_graph_remove_node_empty(Graph& graph) {
	reset_graph(graph);
	const auto [removed, count] = graph.remove_node(1);
	return !removed && count == 0;
}

static bool test_dyn_dir_graph_remove_node_edge(Graph& graph) {
	Graph dg; // Use local isolation
	dg.insert_edge_with_node(1, 2);
	dg.insert_edge_with_node(1, 3);
	dg.insert_edge_with_node(2, 1);
	
	const auto [removed, count] = dg.remove_node(1);
	
	// 1. Check the return values
	if (!removed || count != 3) return false;
	
	// 2. Deep check: Does the node still exist in the map?
	if (dg.node_contains(1)) return false;
	
	// 3. Deep check: Did Node 2 actually erase the incoming edge?
	if (dg.edge_contains(2, 1)) return false;
	
	// 4. Global integrity check
	return dg.checking_edge_integrity().empty();
}

static bool test_dyn_dir_graph_remove_node_normal(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	const auto [removed, count] = graph.remove_node(1);
	return removed && count == 0;
}

static bool test_dyn_dir_graph_insert_edge_empty(Graph& graph) {
	reset_graph(graph);
	return !graph.insert_edge(1, 2);
}

static bool test_dyn_dir_graph_insert_edge_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	return graph.insert_edge(1, 2) && graph.edge_size() == 1;
}

static bool test_dyn_dir_graph_insert_edge_normal(Graph& graph) {
	reset_graph(graph);
	graph.insert_node(1);
	graph.insert_edge(1, 2);
	return !graph.insert_edge(1, 2) && graph.edge_size() == 1;
}

static bool test_dyn_dir_graph_remove_edge_empty(Graph& graph) {
	reset_graph(graph);
	return !graph.remove_edge(1, 2);
}

static bool test_dyn_dir_graph_remove_edge_edge(Graph& graph) {
	build_two_node_one_edge(graph);
	return graph.remove_edge(1, 2) && graph.edge_size() == 0;
}

static bool test_dyn_dir_graph_remove_edge_normal(Graph& graph) {
	build_small_graph(graph);
	return !graph.remove_edge(2, 1);
}

static bool test_dyn_dir_graph_insert_edge_with_node_empty(Graph& graph) {
	reset_graph(graph);
	if (!graph.insert_edge_with_node(1, 2)) {
		return false;
	}
	return graph.node_size() == 2 && graph.edge_size() == 1;
}

static bool test_dyn_dir_graph_insert_edge_with_node_edge(Graph& graph) {
	reset_graph(graph);
	graph.insert_edge_with_node(1, 2);
	return !graph.insert_edge_with_node(1, 2) && graph.edge_size() == 1;
}

static bool test_dyn_dir_graph_insert_edge_with_node_normal(Graph& graph) {
	reset_graph(graph);
	const bool inserted = graph.insert_edge_with_node(1, 1);
	return !inserted && graph.node_size() == 1 && graph.edge_size() == 0;
}

static bool test_dyn_dir_graph_insert_alone_edges_empty(Graph& graph) {
	reset_graph(graph);
	std::vector<Edge> edges;
	const auto uninserted = graph.insert_alone_edges(edges, edges.size());
	return uninserted.empty() && graph.node_size() == 0 && graph.edge_size() == 0;
}

static bool test_dyn_dir_graph_insert_alone_edges_edge(Graph& graph) {
	reset_graph(graph);
	std::vector<Edge> edges;
	edges.emplace_back(1, 2);
	edges.emplace_back(1, 2);
	edges.emplace_back(2, 2);
	const auto uninserted = graph.insert_alone_edges(edges, edges.size());
	return uninserted.size() == 2 && graph.node_size() == 2 && graph.edge_size() == 1;
}

static bool test_dyn_dir_graph_insert_alone_edges_normal(Graph& graph) {
	reset_graph(graph);
	std::vector<Edge> edges;
	edges.emplace_back(1, 2);
	edges.emplace_back(2, 3);
	edges.emplace_back(3, 1);
	const auto uninserted = graph.insert_alone_edges(edges, edges.size());
	return uninserted.empty() && graph.node_size() == 3 && graph.edge_size() == 3;
}

using TestFn = bool (*)(Graph&);

enum class GraphKind {
	Empty,
	Full,
};

struct TestCase {
	const char* name;
	TestFn fn;
	GraphKind kind;
};

static void run_test_case(const TestCase& test, Graph& empty_graph, Graph& full_graph) {
	Graph& graph = (test.kind == GraphKind::Full) ? full_graph : empty_graph;
	if (!test.fn(graph)) {
		std::println("TEST FAILED: {}", test.name);
		std::exit(1);
	}
}

int main(void) {
	Graph full_graph = load_graph_from_file("./Dataset/email_EuAll/Email-EuAll.txt");
	Graph empty_graph{};

	const std::vector<TestCase> tests = {
		{"dyn_node_ctor_empty", test_dyn_node_ctor_empty, GraphKind::Empty},
		{"dyn_node_ctor_edge", test_dyn_node_ctor_edge, GraphKind::Empty},
		{"dyn_node_ctor_normal", test_dyn_node_ctor_normal, GraphKind::Empty},
		{"dyn_node_operator_eq_empty", test_dyn_node_operator_eq_empty, GraphKind::Empty},
		{"dyn_node_operator_eq_edge", test_dyn_node_operator_eq_edge, GraphKind::Empty},
		{"dyn_node_operator_eq_normal", test_dyn_node_operator_eq_normal, GraphKind::Empty},
		{"dyn_node_edge_contains_empty", test_dyn_node_edge_contains_empty, GraphKind::Empty},
		{"dyn_node_edge_contains_edge", test_dyn_node_edge_contains_edge, GraphKind::Empty},
		{"dyn_node_edge_contains_normal", test_dyn_node_edge_contains_normal, GraphKind::Empty},
		{"dyn_node_edges_span_empty", test_dyn_node_edges_span_empty, GraphKind::Empty},
		{"dyn_node_edges_span_edge", test_dyn_node_edges_span_edge, GraphKind::Empty},
		{"dyn_node_edges_span_normal", test_dyn_node_edges_span_normal, GraphKind::Empty},
		{"dyn_node_edges_span_const_empty", test_dyn_node_edges_span_const_empty, GraphKind::Empty},
		{"dyn_node_edges_span_const_edge", test_dyn_node_edges_span_const_edge, GraphKind::Empty},
		{"dyn_node_edges_span_const_normal", test_dyn_node_edges_span_const_normal, GraphKind::Empty},
		{"dyn_node_insert_edge_empty", test_dyn_node_insert_edge_empty, GraphKind::Empty},
		{"dyn_node_insert_edge_edge", test_dyn_node_insert_edge_edge, GraphKind::Empty},
		{"dyn_node_insert_edge_normal", test_dyn_node_insert_edge_normal, GraphKind::Empty},
		{"dyn_node_insert_edge_ref_empty", test_dyn_node_insert_edge_ref_empty, GraphKind::Empty},
		{"dyn_node_insert_edge_ref_edge", test_dyn_node_insert_edge_ref_edge, GraphKind::Empty},
		{"dyn_node_insert_edge_ref_normal", test_dyn_node_insert_edge_ref_normal, GraphKind::Empty},
		{"dyn_node_remove_edge_empty", test_dyn_node_remove_edge_empty, GraphKind::Empty},
		{"dyn_node_remove_edge_edge", test_dyn_node_remove_edge_edge, GraphKind::Empty},
		{"dyn_node_remove_edge_normal", test_dyn_node_remove_edge_normal, GraphKind::Empty},
		{"dyn_node_sort_edge_empty", test_dyn_node_sort_edge_empty, GraphKind::Empty},
		{"dyn_node_sort_edge_edge", test_dyn_node_sort_edge_edge, GraphKind::Empty},
		{"dyn_node_sort_edge_normal", test_dyn_node_sort_edge_normal, GraphKind::Empty},
		{"dyn_dir_graph_ctor_empty", test_dyn_dir_graph_ctor_empty, GraphKind::Empty},
		{"dyn_dir_graph_ctor_edge", test_dyn_dir_graph_ctor_edge, GraphKind::Empty},
		{"dyn_dir_graph_ctor_normal", test_dyn_dir_graph_ctor_normal, GraphKind::Empty},
		{"dyn_dir_graph_node_contains_empty", test_dyn_dir_graph_node_contains_empty, GraphKind::Empty},
		{"dyn_dir_graph_node_contains_edge", test_dyn_dir_graph_node_contains_edge, GraphKind::Empty},
		{"dyn_dir_graph_node_contains_normal", test_dyn_dir_graph_node_contains_normal, GraphKind::Full},
		{"dyn_dir_graph_edge_contains_empty", test_dyn_dir_graph_edge_contains_empty, GraphKind::Empty},
		{"dyn_dir_graph_edge_contains_edge", test_dyn_dir_graph_edge_contains_edge, GraphKind::Empty},
		{"dyn_dir_graph_edge_contains_normal", test_dyn_dir_graph_edge_contains_normal, GraphKind::Full},
		{"dyn_dir_graph_node_size_empty", test_dyn_dir_graph_node_size_empty, GraphKind::Empty},
		{"dyn_dir_graph_node_size_edge", test_dyn_dir_graph_node_size_edge, GraphKind::Empty},
		{"dyn_dir_graph_node_size_normal", test_dyn_dir_graph_node_size_normal, GraphKind::Full},
		{"dyn_dir_graph_edge_size_empty", test_dyn_dir_graph_edge_size_empty, GraphKind::Empty},
		{"dyn_dir_graph_edge_size_edge", test_dyn_dir_graph_edge_size_edge, GraphKind::Empty},
		{"dyn_dir_graph_edge_size_normal", test_dyn_dir_graph_edge_size_normal, GraphKind::Full},
		{"dyn_dir_graph_edge_size_cache_empty", test_dyn_dir_graph_edge_size_cache_empty, GraphKind::Empty},
		{"dyn_dir_graph_edge_size_cache_edge", test_dyn_dir_graph_edge_size_cache_edge, GraphKind::Empty},
		{"dyn_dir_graph_edge_size_cache_normal", test_dyn_dir_graph_edge_size_cache_normal, GraphKind::Full},
		{"dyn_dir_graph_begin_end_empty", test_dyn_dir_graph_begin_end_empty, GraphKind::Empty},
		{"dyn_dir_graph_begin_end_edge", test_dyn_dir_graph_begin_end_edge, GraphKind::Empty},
		{"dyn_dir_graph_begin_end_normal", test_dyn_dir_graph_begin_end_normal, GraphKind::Empty},
		{"dyn_dir_graph_cbegin_cend_empty", test_dyn_dir_graph_cbegin_cend_empty, GraphKind::Empty},
		{"dyn_dir_graph_cbegin_cend_edge", test_dyn_dir_graph_cbegin_cend_edge, GraphKind::Empty},
		{"dyn_dir_graph_cbegin_cend_normal", test_dyn_dir_graph_cbegin_cend_normal, GraphKind::Empty},
		{"dyn_dir_graph_node_range_empty", test_dyn_dir_graph_node_range_empty, GraphKind::Empty},
		{"dyn_dir_graph_node_range_edge", test_dyn_dir_graph_node_range_edge, GraphKind::Empty},
		{"dyn_dir_graph_node_range_normal", test_dyn_dir_graph_node_range_normal, GraphKind::Empty},
		{"dyn_dir_graph_node_range_const_empty", test_dyn_dir_graph_node_range_const_empty, GraphKind::Empty},
		{"dyn_dir_graph_node_range_const_edge", test_dyn_dir_graph_node_range_const_edge, GraphKind::Empty},
		{"dyn_dir_graph_node_range_const_normal", test_dyn_dir_graph_node_range_const_normal, GraphKind::Full},
		{"dyn_dir_graph_node_range_sized_empty", test_dyn_dir_graph_node_range_sized_empty, GraphKind::Empty},
		{"dyn_dir_graph_node_range_sized_edge", test_dyn_dir_graph_node_range_sized_edge, GraphKind::Empty},
		{"dyn_dir_graph_node_range_sized_normal", test_dyn_dir_graph_node_range_sized_normal, GraphKind::Full},
		{"dyn_dir_graph_node_edges_span_empty", test_dyn_dir_graph_node_edges_span_empty, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_span_edge", test_dyn_dir_graph_node_edges_span_edge, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_span_normal", test_dyn_dir_graph_node_edges_span_normal, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_span_const_empty", test_dyn_dir_graph_node_edges_span_const_empty, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_span_const_edge", test_dyn_dir_graph_node_edges_span_const_edge, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_span_const_normal", test_dyn_dir_graph_node_edges_span_const_normal, GraphKind::Full},
		{"dyn_dir_graph_node_edges_range_empty", test_dyn_dir_graph_node_edges_range_empty, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_range_edge", test_dyn_dir_graph_node_edges_range_edge, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_range_normal", test_dyn_dir_graph_node_edges_range_normal, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_range_const_empty", test_dyn_dir_graph_node_edges_range_const_empty, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_range_const_edge", test_dyn_dir_graph_node_edges_range_const_edge, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_range_const_normal", test_dyn_dir_graph_node_edges_range_const_normal, GraphKind::Full},
		{"dyn_dir_graph_node_edges_range_sized_empty", test_dyn_dir_graph_node_edges_range_sized_empty, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_range_sized_edge", test_dyn_dir_graph_node_edges_range_sized_edge, GraphKind::Empty},
		{"dyn_dir_graph_node_edges_range_sized_normal", test_dyn_dir_graph_node_edges_range_sized_normal, GraphKind::Full},
		{"dyn_dir_graph_access_node_ptr_empty", test_dyn_dir_graph_access_node_ptr_empty, GraphKind::Empty},
		{"dyn_dir_graph_access_node_ptr_edge", test_dyn_dir_graph_access_node_ptr_edge, GraphKind::Empty},
		{"dyn_dir_graph_access_node_ptr_normal", test_dyn_dir_graph_access_node_ptr_normal, GraphKind::Full},
		{"dyn_dir_graph_access_node_ptr_const_empty", test_dyn_dir_graph_access_node_ptr_const_empty, GraphKind::Empty},
		{"dyn_dir_graph_access_node_ptr_const_edge", test_dyn_dir_graph_access_node_ptr_const_edge, GraphKind::Empty},
		{"dyn_dir_graph_access_node_ptr_const_normal", test_dyn_dir_graph_access_node_ptr_const_normal, GraphKind::Full},
		{"dyn_dir_graph_checking_edge_integrity_empty", test_dyn_dir_graph_checking_edge_integrity_empty, GraphKind::Empty},
		{"dyn_dir_graph_checking_edge_integrity_edge", test_dyn_dir_graph_checking_edge_integrity_edge, GraphKind::Empty},
		{"dyn_dir_graph_checking_edge_integrity_normal", test_dyn_dir_graph_checking_edge_integrity_normal, GraphKind::Full},
		{"dyn_dir_graph_insert_node_empty", test_dyn_dir_graph_insert_node_empty, GraphKind::Empty},
		{"dyn_dir_graph_insert_node_edge", test_dyn_dir_graph_insert_node_edge, GraphKind::Empty},
		{"dyn_dir_graph_insert_node_normal", test_dyn_dir_graph_insert_node_normal, GraphKind::Empty},
		{"dyn_dir_graph_remove_node_empty", test_dyn_dir_graph_remove_node_empty, GraphKind::Empty},
		{"dyn_dir_graph_remove_node_edge", test_dyn_dir_graph_remove_node_edge, GraphKind::Empty},
		{"dyn_dir_graph_remove_node_normal", test_dyn_dir_graph_remove_node_normal, GraphKind::Empty},
		{"dyn_dir_graph_insert_edge_empty", test_dyn_dir_graph_insert_edge_empty, GraphKind::Empty},
		{"dyn_dir_graph_insert_edge_edge", test_dyn_dir_graph_insert_edge_edge, GraphKind::Empty},
		{"dyn_dir_graph_insert_edge_normal", test_dyn_dir_graph_insert_edge_normal, GraphKind::Empty},
		{"dyn_dir_graph_remove_edge_empty", test_dyn_dir_graph_remove_edge_empty, GraphKind::Empty},
		{"dyn_dir_graph_remove_edge_edge", test_dyn_dir_graph_remove_edge_edge, GraphKind::Empty},
		{"dyn_dir_graph_remove_edge_normal", test_dyn_dir_graph_remove_edge_normal, GraphKind::Empty},
		{"dyn_dir_graph_insert_edge_with_node_empty", test_dyn_dir_graph_insert_edge_with_node_empty, GraphKind::Empty},
		{"dyn_dir_graph_insert_edge_with_node_edge", test_dyn_dir_graph_insert_edge_with_node_edge, GraphKind::Empty},
		{"dyn_dir_graph_insert_edge_with_node_normal", test_dyn_dir_graph_insert_edge_with_node_normal, GraphKind::Empty},
		{"dyn_dir_graph_insert_alone_edges_empty", test_dyn_dir_graph_insert_alone_edges_empty, GraphKind::Empty},
		{"dyn_dir_graph_insert_alone_edges_edge", test_dyn_dir_graph_insert_alone_edges_edge, GraphKind::Empty},
		{"dyn_dir_graph_insert_alone_edges_normal", test_dyn_dir_graph_insert_alone_edges_normal, GraphKind::Empty},
	};

	for (const auto& test : tests) {
		run_test_case(test, empty_graph, full_graph);
	}

	std::println("All tests passed.");
	return 0;
}
