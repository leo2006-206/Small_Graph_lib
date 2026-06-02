import std;


int main(void){

	// std::vector v{1, 2, 4, 5};

	// std::print("vector is {}\n", v);

	// auto it = std::ranges::lower_bound(v, 3);

	// auto it2 = v.emplace(it, 3);

	// std::print("vector is {}\n", v);

	// *it2 = 6;

	// std::print("vector is {}\n", v);

	// auto& ref = v.emplace_back(7);

	// std::print("vector is {}\n", v);

	// ref = 8;

	// std::print("vector is {}\n", v);

	// Create a completely empty dummy hasher just to satisfy the template for sizeof
	struct dummy_hasher {
		std::size_t operator()(const std::pair<int, int>&) const { return 0; }
	};

	// Then change your print line to this:
	std::print("\nset = {}, map = {}", 
		sizeof(std::unordered_set<std::pair<int, int>, dummy_hasher>), 
		sizeof(std::unordered_map<int, int>));
}