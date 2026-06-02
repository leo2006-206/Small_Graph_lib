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

	std::set<int> s;

	s.emplace(std::numeric_limits<int>::max());
	s.emplace(std::numeric_limits<int>::min());

	for(auto i : s){
		std::cout<<(std::bitset<32>(i))<<"\t"<<i<<"\n";
	}
}