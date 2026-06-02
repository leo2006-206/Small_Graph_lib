import std;
import Graph;

auto load_graph_from_file(const std::string& filename) {
    Small_Graph::dyn_dir_graph dg;
	std::vector<Small_Graph::alone_edge> al_vec;
	al_vec.reserve(420045);

    std::ifstream file(filename);
    std::string line;
	

    while (std::getline(file, line)) {
        // 1. Skip comments
        if (line.empty() || line[0] == '%' || line[0] == '#') continue;

        // 2. Convert commas to spaces so stringstream can parse it easily
        std::replace(line.begin(), line.end(), ',', ' ');
		std::istringstream iss(line);

		Small_Graph::node_id_t source, dest;

		if(iss >> source >> dest){
			al_vec.emplace_back(source, dest);
		}
    }

	auto fail_edge_vec = dg.insert_alone_edges(al_vec, al_vec.capacity());

	std::println("al_vec size = {}", al_vec.size());
	std::println("map node size = {}", dg.node_size());
	std::println("map edge size = {}", dg.edge_size_cache());
	std::println("fail_edge_vec size = {}", fail_edge_vec.size());

	return dg;
}

int main(void){
	auto g = load_graph_from_file("./Dataset/email_EuAll/Email-EuAll.txt");
}