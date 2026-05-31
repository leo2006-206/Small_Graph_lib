import std;

struct node{
	int id;
	float wei;
};


int main(void){

	std::vector<node> v{
		{1, 5.5}, 
        {3, 2.1}, 
        {5, 9.9}, 
        {8, 1.0}
	};

	auto it = std::ranges::lower_bound(v, 5, std::less<>{}, &node::id);

	if (it != v.end() && it->id == 5) {
		// Found it
		std::print("find\n");
	}
}