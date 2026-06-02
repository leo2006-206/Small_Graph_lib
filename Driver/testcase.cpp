// import std;
// import Helper;
// import Graph;

// using namespace Small_Graph::dynamic_graph;

// struct TestContext {
// 	int failures = 0;

// 	void check(bool condition, const char* message) {
// 		if (!condition) {
// 			++failures;
// 			std::cout << "FAIL: " << message << "\n";
// 		}
// 	}
// };

// void test_empty_graph_initialization(TestContext& ctx) {
// 	dyn_dir_graph g;
// 	ctx.check(g.node_size() == 0, "empty graph node_size is 0");
// 	ctx.check(!g.node_contains(42), "empty graph node_contains returns false");

// 	std::size_t count = 0;
// 	for (const auto& node : g.node_range()) {
// 		(void)node;
// 		++count;
// 	}
// 	ctx.check(count == 0, "empty graph node_range iterates zero times");
// }

// void test_unordered_insertion_duplication(TestContext& ctx) {
// 	dyn_dir_graph g;
// 	ctx.check(g.insert_node_unord(3) != nullptr, "insert_node_unord inserts 3");
// 	ctx.check(g.insert_node_unord(1) != nullptr, "insert_node_unord inserts 1");
// 	ctx.check(g.insert_node_unord(2) != nullptr, "insert_node_unord inserts 2");
// 	ctx.check(g.node_size() == 3, "node_size is 3 after unordered inserts");
// 	ctx.check(g.insert_node_unord(1) == nullptr, "insert_node_unord rejects duplicate 1");
// 	ctx.check(g.node_size() == 3, "node_size stays 3 after duplicate insert");
// }

// void test_ordered_insertion_sorting(TestContext& ctx) {
// 	dyn_dir_graph g;
// 	g.insert_node_unord(5);
// 	g.insert_node_unord(1);
// 	g.insert_node_unord(4);

// 	g.sort_node();
// 	ctx.check(g.insert_node_ord(3) != nullptr, "insert_node_ord inserts 3 after sort");

// 	std::vector<node_id_t> ids;
// 	for (const auto& node : g.node_range()) {
// 		ids.push_back(node._id);
// 	}
// 	ctx.check(ids.size() == 4, "node_range yields 4 nodes after insertions");
// 	ctx.check(ids[0] == 1 && ids[1] == 3 && ids[2] == 4 && ids[3] == 5,
// 		"node_range yields nodes in ascending order");
// }

// void test_isolated_node_removal(TestContext& ctx) {
// 	dyn_dir_graph g;
// 	g.insert_node_ord(10);
// 	ctx.check(g.node_contains(10), "node 10 exists before removal");
// 	ctx.check(g.remove_node(10), "remove_node returns true for existing node");
// 	ctx.check(!g.node_contains(10), "node 10 no longer exists after removal");
// 	ctx.check(!g.remove_node(10), "remove_node returns false for missing node");
// }

// void test_valid_edge_insertion(TestContext& ctx) {
// 	dyn_dir_graph g;
// 	const node_id_t a = 100;
// 	const node_id_t b = 200;

// 	g.insert_node_ord(a);
// 	g.insert_node_ord(b);

// 	ctx.check(g.insert_edge(a, b), "insert_edge returns true for valid edge");
// 	ctx.check(g.edge_contains(a, b), "edge_contains true for A->B");
// 	ctx.check(!g.edge_contains(b, a), "edge_contains false for B->A (directed)");

// 	const auto edges = g.node_edges_range(a);
// 	ctx.check(edges.size() == 1, "node_edges_range size is 1 for A");
// 	ctx.check(edges[0] == b, "node_edges_range contains B for A");
// }

// void test_invalid_edge_insertion_missing_nodes(TestContext& ctx) {
// 	dyn_dir_graph g;
// 	const node_id_t a = 1;
// 	const node_id_t b = 2;
// 	const node_id_t c = 3;

// 	g.insert_node_ord(a);
// 	ctx.check(!g.insert_edge(a, b), "insert_edge returns false if dest missing");
// 	ctx.check(!g.insert_edge(c, a), "insert_edge returns false if source missing");
// }

// void test_edge_duplication_removal(TestContext& ctx) {
// 	dyn_dir_graph g;
// 	const node_id_t a = 10;
// 	const node_id_t b = 20;

// 	g.insert_node_ord(a);
// 	g.insert_node_ord(b);

// 	ctx.check(g.insert_edge(a, b), "insert_edge inserts A->B");
// 	ctx.check(!g.insert_edge(a, b), "insert_edge rejects duplicate A->B");
// 	ctx.check(g.remove_edge(a, b), "remove_edge returns true for existing edge");
// 	ctx.check(!g.remove_edge(a, b), "remove_edge returns false for missing edge");
// }

// void test_self_loops_cycles(TestContext& ctx) {
// 	dyn_dir_graph g;
// 	const node_id_t a = 1;
// 	const node_id_t b = 2;
// 	const node_id_t c = 3;

// 	g.insert_node_ord(a);
// 	g.insert_node_ord(b);
// 	g.insert_node_ord(c);

// 	ctx.check(g.insert_edge(a, b), "insert_edge A->B for cycle");
// 	ctx.check(g.insert_edge(b, c), "insert_edge B->C for cycle");
// 	ctx.check(g.insert_edge(c, a), "insert_edge C->A for cycle");
// 	ctx.check(g.insert_edge(a, a), "insert_edge allows self-loop A->A");
// 	ctx.check(g.edge_contains(a, a), "edge_contains true for self-loop A->A");

// 	const auto edges = g.node_edges_range(a);
// 	bool has_b = false;
// 	bool has_a = false;
// 	for (const auto dest : edges) {
// 		if (dest == b) {
// 			has_b = true;
// 		} else if (dest == a) {
// 			has_a = true;
// 		}
// 	}
// 	ctx.check(edges.size() == 2, "node_edges_range A has exactly 2 edges");
// 	ctx.check(has_b && has_a, "node_edges_range A has B and A destinations");
// }

// void test_dangling_edge_deletion(TestContext& ctx) {
// 	dyn_dir_graph g;
// 	const node_id_t a = 1;
// 	const node_id_t b = 2;
// 	const node_id_t c = 3;

// 	g.insert_node_ord(a);
// 	g.insert_node_ord(b);
// 	g.insert_node_ord(c);

// 	g.insert_edge(a, b);
// 	g.insert_edge(c, b);

// 	ctx.check(g.remove_node(b), "remove_node removes node with incoming edges");
// 	ctx.check(!g.edge_contains(a, b), "edge A->B removed after deleting B");
// 	ctx.check(!g.edge_contains(c, b), "edge C->B removed after deleting B");
// }

// void verify_const_accessors(const dyn_dir_graph& g, TestContext& ctx) {
// 	std::size_t node_count = 0;
// 	for (const auto& node : g.node_range()) {
// 		(void)node;
// 		++node_count;
// 	}
// 	ctx.check(node_count == g.node_size(), "const node_range iterates all nodes");

// 	const auto* node_ptr = g.access_node_ptr(1);
// 	if (node_ptr != nullptr) {
// 		const auto edges = g.node_edges_range(node_ptr->_id);
// 		std::size_t edge_count = 0;
// 		for (const auto edge_dest : edges) {
// 			(void)edge_dest;
// 			++edge_count;
// 		}
// 		ctx.check(edge_count == edges.size(), "const node_edges_range returns valid span");
// 	} else {
// 		ctx.check(false, "const access_node_ptr returns non-null for existing node");
// 	}
// }

// void test_const_correctness_readonly(TestContext& ctx) {
// 	dyn_dir_graph g;
// 	g.insert_node_ord(1);
// 	g.insert_node_ord(2);
// 	g.insert_edge(1, 2);

// 	verify_const_accessors(g, ctx);
// }

int main(void){
// 	TestContext ctx;

// 	test_empty_graph_initialization(ctx);
// 	test_unordered_insertion_duplication(ctx);
// 	test_ordered_insertion_sorting(ctx);
// 	test_isolated_node_removal(ctx);
// 	test_valid_edge_insertion(ctx);
// 	test_invalid_edge_insertion_missing_nodes(ctx);
// 	test_edge_duplication_removal(ctx);
// 	test_self_loops_cycles(ctx);
// 	test_dangling_edge_deletion(ctx);
// 	test_const_correctness_readonly(ctx);

// 	if (ctx.failures == 0) {
// 		std::cout << "All tests passed.\n";
// 	} else {
// 		std::cout << "Tests failed: " << ctx.failures << "\n";
// 	}
// 	return ctx.failures == 0 ? 0 : 1;
}