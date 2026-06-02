import std;
import Graph;

auto load_graph_from_file(const std::string& filename) {
    Small_Graph::dyn_dir_graph dg;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        // 1. Skip comments
        if (line.empty() || line[0] == '%') continue;

        // 2. Convert commas to spaces so stringstream can parse it easily
        std::replace(line.begin(), line.end(), ',', ' ');

		std::println("{}", line);
    }

	return dg;
}

int main(void){
	auto g = load_graph_from_file("./Dataset/advogato/advogato.edges");
}