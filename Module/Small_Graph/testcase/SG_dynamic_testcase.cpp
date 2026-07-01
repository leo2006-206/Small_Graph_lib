import std;
import Small_Graph;

void test_dyn_node() {
    using namespace SG;
	auto print_dyn_node = [](dyn_node n){
		std::println("source = {}, |E| = {}", n.source_id_, n.edge_vec_.size());
		for(auto&& ae : n.alone_edges_range()){
			std::println("\t{}", ae);
		}
	};

    dyn_node node(1);

    // 1. Explicitly test self-loop rejection (Assuming source_id_ == 1 rejects edge 1)
    bool self_insert = node.insert_edge(1);
    assert(self_insert == false && "Self-loop insertion should fail");

    // 2. Insert valid edges (2 through 9)
    for (node_id_t i : std::ranges::iota_view(2uz, 10uz)) {
        bool success = node.insert_edge(i);
        assert(success == true);
    }

    // 3. Test edge containment
    assert(node.edge_contains(10) == false);
    assert(node.edge_contains(5) == true);

    // 4. Verify spans and views
    for (const auto out_edge : node.edges_span()) {
        assert(out_edge >= 2 && out_edge <= 9);
    }

    for (const auto edge : node.alone_edges_range()) {
        assert(edge.dist >= 2 && edge.dist <= 9);
        // Bonus: Actually check that your make_alone transform worked!
        assert(edge.source == 1); 
    }

    // 5. Test insertions (Safe from the NDEBUG assert trap)
    bool insert_0 = node.insert_edge(0);
    assert(insert_0 == true);

    bool try_insert_0 = node.try_insert_edge(0);
    assert(try_insert_0 == false && "Should not insert duplicate edge");

    // 6. Test removals
    bool remove_0 = node.remove_edge(0);
    assert(remove_0 == true);

    bool try_remove_0 = node.try_remove_edge(0); // You forgot to test this one!
    assert(try_remove_0 == false && "Should return false when removing non-existent edge");

    // 7. Test sorting functionality using standard algorithms
    node.sort_edges();
    
    // Much safer and cleaner than using any_of with a stateful lambda
    assert(std::ranges::is_sorted(node.edges_span()));

    // 8. Test equality operator (This was missing in your original test)
    dyn_node node_same(1);
    dyn_node node_different(42);
    assert(node == node_same);
    assert(!(node == node_different));

	//9.  Test for range insert and remove
	std::vector<node_id_t> dist_vec{10, 11, 12, 10, 1};
	//true, true, true, false, false

	auto insert_failed_count = node.insert_range_edges(dist_vec);
	assert(insert_failed_count == 2);

	auto remove_failed_count = node.remove_range_edges(dist_vec);
	assert(remove_failed_count == 2);

	node.insert_range_edges(dist_vec);
	auto failed_insert_dist = node.insert_range_edges_vec(dist_vec, 10);
	assert(failed_insert_dist == dist_vec);

	auto failed_remove_dist = node.remove_range_edges_vec(dist_vec, 10);
	std::vector<node_id_t> ff_vec;
	ff_vec.emplace_back(10);ff_vec.emplace_back(1);
	assert(failed_remove_dist == ff_vec);

	print_dyn_node(node);

}

void test_dyn_graph(){
    using namespace SG;

    dyn_graph graph;

    // The raw input data
    std::vector<alone_edge> input_edges = {
        alone_edge{1, 2}, alone_edge{1, 3},
        alone_edge{1, 4}, alone_edge{1, 5},
        alone_edge{2, 1}, alone_edge{2, 3},
        alone_edge{1, 2}, // duplicate (fail 1)
        alone_edge{1, 1}, // self-loop (fail 2)
        alone_edge{2, 2}  // self-loop (fail 3)
    };

    // 1. Test batch insertion and failure counts
    auto failed_count = graph.insert_range(input_edges);
    assert(failed_count == 3 && "Should fail on exactly 1 duplicate and 2 self-loops");

    // 2. Explicitly test that valid edges exist
    std::vector<alone_edge> expected_valid = {
        alone_edge{1, 2},
		alone_edge{1, 3},
		alone_edge{1, 4},
		alone_edge{1, 5}, 
		alone_edge{2, 1}, 
		alone_edge{2, 3}
    };
    for(const auto& edge : expected_valid){
        assert(graph.edge_contains(edge) && "Valid edge should be in the graph");
    }

    // 3. Explicitly test that invalid/uninserted edges DO NOT exist
    assert(!graph.edge_contains(alone_edge{1, 1}) && "Self loops should not exist");
    assert(!graph.edge_contains(alone_edge{2, 2}) && "Self loops should not exist");
    
    // (Testing directed nature: 1->4 exists, but 4->1 was never added)
    assert(!graph.edge_contains(alone_edge{4, 1}) && "Graph should be directed; reverse edge shouldn't exist");
    
    // (Testing completely unknown nodes)
    assert(!graph.edge_contains(alone_edge{99, 100}) && "Completely fake edges should return false");

    // 4. Test Node existence (Positive space)
    for(node_id_t i : std::views::iota(1uz, 6uz)){
        assert(graph.node_contains(i) && "Nodes 1-5 should exist");
        assert(graph.find_node(i) != graph.node_table.end());
    }

    // 5. Test Node non-existence (Negative space - VERY IMPORTANT)
    assert(!graph.node_contains(6) && "Node 6 was never added");
    assert(graph.find_node(6) == graph.node_table.end());
    assert(!graph.node_contains(99) && "Node 99 was never added");

}

using namespace SG::dynamic_graph;

void test_packing_node_id_map() {
    dyn_graph graph;

    // 1. Setup a sparse graph with gaps in IDs
    // Node 10 points to 20 and 30
    graph.insert_edge(10, 20);
    graph.insert_edge(10, 30);
    // Node 20 points to 30
    graph.insert_edge(20, 30);
    // Node 30 points to 10
    graph.insert_edge(30, 10);

    // 2. Perform packing
    auto id_map = graph.packing_node_id_map();

    // 3. Verify the returned Map (Old ID -> New ID)
    assert(id_map.size() == 3);
    assert(id_map.at(10) == 0);
    assert(id_map.at(20) == 1);
    assert(id_map.at(30) == 2);

    // 4. Verify internal node keys are updated
    assert(graph.node_table_.size() == 3);
    assert(graph.node_contains(0) == true);
    assert(graph.node_contains(1) == true);
    assert(graph.node_contains(2) == true);
    assert(graph.node_contains(10) == false); // Old ID should be gone

    // 5. Verify internal edges are updated
    // Old Node 10 (now 0) should point to Old Node 20 (now 1) and 30 (now 2)
    auto edges_0 = graph.node_edges_span(0);
    assert(edges_0.size() == 2);
    // Note: If edge_vec_ isn't strictly sorted during insert, you may need to search. 
    // Assuming we just check if they exist:
    assert(graph.edge_contains(0, 1) == true);
    assert(graph.edge_contains(0, 2) == true);

    // Old Node 20 (now 1) should point to Old Node 30 (now 2)
    assert(graph.edge_contains(1, 2) == true);
    assert(graph.edge_contains(1, 3) == false);

    std::cout << "[PASS] test_packing_node_id_map\n";
}

void test_packing_node_id_vec() {
    dyn_graph graph;

    // 1. Setup sparse graph
    graph.insert_edge(100, 500);
    graph.insert_edge(500, 100);
    graph.insert_edge(500, 900);

    // 2. Perform packing
    auto id_vec = graph.packing_node_id_vec();

    // 3. Verify the returned Vector (New ID -> Old ID)
    // Index is New ID, Value is Old ID
    assert(id_vec.size() == 3);
    assert(id_vec[0] == 100);
    assert(id_vec[1] == 500);
    assert(id_vec[2] == 900);

    // 4. Verify internal edges are correctly mapped via std::distance
    // Old Node 500 (now 1) should point to Old Node 100 (now 0) and 900 (now 2)
    assert(graph.edge_contains(1, 0) == true);
    assert(graph.edge_contains(1, 2) == true);
    assert(graph.edge_contains(1, 500) == false); // ensure old IDs are wiped from edges

    std::cout << "[PASS] test_packing_node_id_vec\n";
}

void test_already_packed_optimization() {
    dyn_graph graph;

    // 1. Setup a perfectly packed graph [0, 1, 2]
    graph.insert_edge(0, 1);
    graph.insert_edge(1, 2);
    graph.insert_edge(2, 0);

    // 2. Perform packing
    auto id_vec = graph.packing_node_id_vec();

    // 3. Verify early exit (should return empty vector as no packing was needed)
    assert(id_vec.empty() == true);

    // 4. Verify graph remains fully intact
    assert(graph.node_contains(0) == true);
    assert(graph.node_contains(1) == true);
    assert(graph.node_contains(2) == true);
    assert(graph.edge_contains(0, 1) == true);

    std::cout << "[PASS] test_already_packed_optimization\n";
}