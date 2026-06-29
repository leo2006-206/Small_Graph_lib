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