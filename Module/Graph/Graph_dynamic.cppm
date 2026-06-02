module;

export module Graph:dynamic_graph;

import std;
import :function;

export namespace Small_Graph::dynamic_graph{
	using node_id_t = std::uint64_t;

	constexpr node_id_t get_invalid_node(){
		return std::numeric_limits<node_id_t>::max();
	}

	struct alone_edge;
	struct dyn_node;
	struct dyn_dir_graph;
}

namespace std{

template<>
struct hash<Small_Graph::dynamic_graph::dyn_node>{
	std::size_t operator()(const Small_Graph::dynamic_graph::dyn_node& node) const noexcept;
};

}

export namespace Small_Graph::dynamic_graph{

struct alone_edge{
	node_id_t	_source;
	node_id_t	_dest;

	alone_edge() = delete;
	constexpr alone_edge(node_id_t source, node_id_t dest)
	: _source(source), _dest(dest){}

	constexpr static alone_edge get_invalid(){
		constexpr auto lim = get_invalid_node();
		return {lim, lim};
	}
};

struct dyn_node{
	using edge_vec_t = std::vector<node_id_t /*edge dest*/>;

	node_id_t	_id;
	edge_vec_t	_edge_vec;

	dyn_node() = delete;
	constexpr dyn_node(node_id_t id) : _id(id){};

	constexpr bool operator==(const dyn_node& other) const{
		return (_id == other._id);
	}

	bool						edge_contains(node_id_t dest) const;

	std::span<node_id_t>		edges_span();
	std::span<const node_id_t>	edges_span() const;

	bool						insert_edge(node_id_t dest);
	bool						remove_edge(node_id_t dest);
	void						sort_edge();
};

struct dyn_dir_graph{
	using node_t				= dyn_node;
	using node_map_t			= std::unordered_map<
		node_id_t,	//key
		dyn_node,	//value
		std::hash<node_id_t>,
		std::equal_to<node_id_t>,
		std::allocator<std::pair<const node_id_t, dyn_node>>
	>;
	using node_map_it_t			= node_map_t::iterator;
	using node_map_cit_t		= node_map_t::const_iterator;

	using node_map_pair_range_t	= std::ranges::subrange<node_map_t::iterator>;
	using node_map_pair_crange_t= std::ranges::subrange<node_map_t::const_iterator>;
	using node_map_pair_srange_t= node_map_pair_crange_t;	//sized range, const by design

	using node_range_t			= std::ranges::elements_view<node_map_pair_range_t, 1>;
	using node_crange_t			= std::ranges::elements_view<node_map_pair_crange_t, 1>;
	using node_srange_t			= std::ranges::elements_view<node_map_pair_srange_t, 1>;	//sized range, const by design

	using edge_range_t			= std::ranges::subrange<std::span<node_id_t>::iterator>;
	using edge_crange_t			= std::ranges::subrange<std::ranges::iterator_t<std::span<const node_id_t>>>; //clang didnt support const_iterator
	using edge_srange_t			= edge_crange_t;


	node_map_t					_node_map;

	bool						node_contains(node_id_t id) const;
	bool						edge_contains(node_id_t source, node_id_t dest) const;

	std::size_t					node_size() const;

	node_map_it_t				begin();
	node_map_it_t				end();
	node_map_cit_t				cbegin();
	node_map_cit_t				cend();

	node_map_pair_range_t		pair_range();
	node_map_pair_crange_t		pair_range() const;
	node_map_pair_srange_t		pair_range_sized() const;

	node_range_t				node_range();
	node_crange_t				node_range() const;
	node_srange_t				node_range_sized() const;

	std::span<node_id_t>		node_edges_span(node_id_t id);
	std::span<const node_id_t>	node_edges_span(node_id_t id) const;

	edge_range_t				node_edges_range(node_id_t id);				//aka range{span}
	edge_crange_t				node_edges_range(node_id_t id) const;		//aka range{span}
	edge_srange_t				node_edges_range_sized(node_id_t id) const;	//aka range{span, span.size()}

	node_t*						access_node_ptr(node_id_t id);
	const node_t*				access_node_ptr(node_id_t id) const;

	node_t*						insert_node(node_id_t id);
	std::pair<bool, std::size_t>remove_node(node_id_t id);	//bool= removed, size_t= num removed incoming edge to id

	bool						insert_edge(node_id_t source, node_id_t dest);
	bool						remove_edge(node_id_t source, node_id_t dest);

	bool						insert_edge_with_node(node_id_t source, node_id_t dest); //if source not exit, insert node(source)

	template <std::ranges::input_range R>
    requires std::same_as<std::ranges::range_value_t<R>, alone_edge>
	std::vector<alone_edge>		insert_alone_edges(const R& edges_range, std::size_t reserve_size); //return the vec of edge exist in graph
};

}

namespace Small_Graph::dynamic_graph {

// dyn_node start here

bool dyn_node::edge_contains(node_id_t dest) const{
	for(const auto edge_dest : _edge_vec){
		if(dest == edge_dest){
			return true;
		}
	}
	return false;
}

std::span<node_id_t> dyn_node::edges_span(){
	return (_edge_vec);
}

std::span<const node_id_t> dyn_node::edges_span() const{
	return (_edge_vec);
}

bool dyn_node::insert_edge(node_id_t dest){
	if(dest == _id || edge_contains(dest) == true){
		return false;
	}
	_edge_vec.emplace_back(dest);
	return true;
}

bool dyn_node::remove_edge(node_id_t dest){
	return (std::erase(_edge_vec, dest) > 0);
}

void dyn_node::sort_edge(){
	auto cmp = [](node_id_t a, node_id_t b)-> bool{
		return a > b;
	};

	std::ranges::sort(_edge_vec, cmp);
}

// dyn_dir_graph start here

bool dyn_dir_graph::node_contains(node_id_t id) const{
	return (access_node_ptr(id) == nullptr);
}
bool dyn_dir_graph::edge_contains(node_id_t source, node_id_t dest) const{
	const auto ptr = access_node_ptr(source);
	if(ptr == nullptr){
		return false;
	}

	return ptr->edge_contains(dest);
}
std::size_t dyn_dir_graph::node_size() const{
	return _node_map.size();
}
dyn_dir_graph::node_map_it_t dyn_dir_graph::begin(){
	return _node_map.begin();
}
dyn_dir_graph::node_map_it_t dyn_dir_graph::end(){
	return _node_map.end();
}
dyn_dir_graph::node_map_cit_t dyn_dir_graph::cbegin(){
	return _node_map.cbegin();
}
dyn_dir_graph::node_map_cit_t dyn_dir_graph::cend(){
	return _node_map.cend();
}
dyn_dir_graph::node_map_pair_range_t dyn_dir_graph::pair_range(){
	return std::ranges::subrange{_node_map.begin(), _node_map.end()};
}
dyn_dir_graph::node_map_pair_crange_t dyn_dir_graph::pair_range() const{
	return std::ranges::subrange{_node_map.cbegin(), _node_map.cend()};
}
dyn_dir_graph::node_map_pair_srange_t dyn_dir_graph::pair_range_sized() const{
	return std::ranges::subrange{_node_map.cbegin(), _node_map.cend(), _node_map.size()};
}
dyn_dir_graph::node_range_t dyn_dir_graph::node_range(){
	return pair_range() | std::views::values;
}
dyn_dir_graph::node_crange_t dyn_dir_graph::node_range() const{
	return pair_range() | std::views::values;
}
dyn_dir_graph::node_srange_t dyn_dir_graph::node_range_sized() const{
	return pair_range_sized() | std::views::values;
}
std::span<node_id_t> dyn_dir_graph::node_edges_span(node_id_t id){
	auto ptr = access_node_ptr(id);
	if(ptr == nullptr){
		return {};
	}
	return ptr->edges_span();
}
std::span<const node_id_t> dyn_dir_graph::node_edges_span(node_id_t id) const{
	const auto ptr = access_node_ptr(id);
	if(ptr == nullptr){
		return {};
	}
	return ptr->edges_span();
}
dyn_dir_graph::edge_range_t dyn_dir_graph::node_edges_range(node_id_t id){
	return std::ranges::subrange{node_edges_span(id)};
}
dyn_dir_graph::edge_crange_t dyn_dir_graph::node_edges_range(node_id_t id) const{
	return std::ranges::subrange{node_edges_span(id)};
}
dyn_dir_graph::edge_srange_t dyn_dir_graph::node_edges_range_sized(node_id_t id) const{
	const auto edge_span = node_edges_span(id);
	return std::ranges::subrange{edge_span, edge_span.size()};
}
dyn_dir_graph::node_t* dyn_dir_graph::access_node_ptr(node_id_t id){
	auto it = _node_map.find(id);
	if(it == _node_map.end()){
		return nullptr;
	}
	return &(it->second);
}
const dyn_dir_graph::node_t* dyn_dir_graph::access_node_ptr(node_id_t id) const{
	const auto it = _node_map.find(id);
	if(it == _node_map.cend()){
		return nullptr;
	}
	return &(it->second);
}
dyn_dir_graph::node_t* dyn_dir_graph::insert_node(node_id_t id){
	auto [it, inserted] = _node_map.try_emplace(id, id);
	if(inserted){
		return &(it->second);
	}
	return nullptr;
}
std::pair<bool, std::size_t> dyn_dir_graph::remove_node(node_id_t id){
	auto removed = _node_map.erase(id);
	if(removed == 0){
		return {false, {}};
	}

	std::size_t num_incoming_edge{};
	for(auto& node : node_range()){
		num_incoming_edge += static_cast<std::size_t>(node.remove_edge(id));
	}

	return {true, num_incoming_edge};
}
bool dyn_dir_graph::insert_edge(node_id_t source, node_id_t dest){
	auto ptr = access_node_ptr(source);
	if(ptr == nullptr){
		return false;
	}
	return ptr->insert_edge(dest);
}
bool dyn_dir_graph::remove_edge(node_id_t source, node_id_t dest){
	auto ptr = access_node_ptr(source);
	if(ptr == nullptr){
		return false;
	}
	return ptr->remove_edge(dest);
}
bool dyn_dir_graph::insert_edge_with_node(node_id_t source, node_id_t dest){
	auto [it, inserted] = _node_map.try_emplace(source, source);
	return it->second.insert_edge(dest);
}
template <std::ranges::input_range R>
requires std::same_as<std::ranges::range_value_t<R>, alone_edge>
std::vector<alone_edge> dyn_dir_graph::insert_alone_edges(const R& edges_range, std::size_t reserve_size){
	_node_map.reserve(reserve_size);
	std::vector<alone_edge> uninserted_edge_vec;
	uninserted_edge_vec.reserve(1024);

	for(alone_edge al_edge : edges_range){
		bool inserted = insert_edge_with_node(al_edge._source, al_edge._dest);
		if(inserted == false){
			uninserted_edge_vec.push_back(al_edge);
		}
	}
	return uninserted_edge_vec;
}

}

std::size_t
std::hash<Small_Graph::dynamic_graph::dyn_node>::operator()
(const Small_Graph::dynamic_graph::dyn_node& node) const noexcept{
	return std::hash<Small_Graph::dynamic_graph::node_id_t>{}(node._id);
}