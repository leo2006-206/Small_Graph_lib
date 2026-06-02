module;

export module Graph:dynamic_graph;

import std;
import :function;

export namespace Small_Graph::dynamic_graph{
	using node_id_t		= std::int64_t;

	static_assert(
		std::is_signed_v<node_id_t> && std::is_integral_v<node_id_t>,
		"node_dir_id_t required to be signed, where negative = outgoing edge, positive = incoming edge"
	);

	// negative = outgoing edge
	// positive = incoming edge
	// because 0 == -0, cant represent negative/positive => outgoing/incoming
	// so 0 is invalid value, in both outgoing and incoming
	// so add 1 to id, that 0 + 1 = 1, then -1/+1
	// so -1 => outgoing edge to	0
	// so +1 => incoming edge form	0

	enum class edge_dir_t : bool{
		outgoing_flag,
		incoming_flag
	};
	constinit edge_dir_t int_flag = edge_dir_t::incoming_flag;
	// as id or size_t, aka int64_t

	struct node_dir_id_t{
		private:
		node_id_t _raw_id{0}; 

		public:
		constexpr node_dir_id_t() = default;
		constexpr node_dir_id_t(edge_dir_t flag, node_id_t id){
			if (flag == edge_dir_t::outgoing_flag){
				_raw_id = -(id + 1);
			}
			else/* if(flag == edge_dir_t::incoming_flag)*/{
				_raw_id = +(id + 1);
			}
		}

		constexpr bool operator==(const node_dir_id_t&) const = default;
		constexpr bool is_valid() const{
			return (_raw_id != 0);
		}
		constexpr std::pair<edge_dir_t, node_id_t> unpack() const{
			if(_raw_id > 0){
				return {edge_dir_t::incoming_flag, _raw_id - 1};
			}
			else if(_raw_id < 0){
				return {edge_dir_t::outgoing_flag, std::abs(_raw_id) - 1};
			}
			return {{}/*invalid in both direction*/, 0};
		}
		constexpr node_id_t unpack_int(){
			// if this node_dir_id only store the node id
			return _raw_id - 1;
		}
	};

	struct alone_edge;
	struct dyn_dir_node;
	struct dyn_dir_graph;
}

namespace std{

template<>
struct hash<Small_Graph::dynamic_graph::dyn_dir_node>{
	std::size_t operator()(const Small_Graph::dynamic_graph::dyn_dir_node& node) const noexcept;
};

}

export namespace Small_Graph::dynamic_graph{

struct alone_edge{
	node_id_t	_source;
	node_id_t	_dest;

	alone_edge() = delete;
	constexpr alone_edge(node_id_t source, node_id_t dest)
	: _source(source), _dest(dest){}

	// constexpr static alone_edge get_invalid(){
	// 	constexpr auto lim = get_invalid_node();
	// 	return {lim, lim};
	// }
};

struct dyn_dir_node{
	using edge_vec_t = std::vector<node_dir_id_t /*edge dest*/>;

	edge_vec_t	_edge_vec;
	std::size_t	_num_outgoing;
	// id				= edge_vex[0].unpack_int();
	// num total edge	= edge_vec.size()
	// num outgoing		= _num_outgoing
	// num incoming 	= edge_vec.size() - edge_vec[1]

	dyn_dir_node() = delete;
	constexpr dyn_dir_node(node_id_t id, std::size_t reserve_size = 64)
	: _edge_vec(){
		_edge_vec.reserve(reserve_size);
		_edge_vec.emplace_back(int_flag, id);
		// storing the actaul id, no direction
	};


	constexpr bool operator==(const dyn_dir_node& other) const{
		return (_edge_vec[0] == other._edge_vec[0]);
	}

	node_id_t&						id();
	const node_id_t&				id() const;

	std::size_t						degree() const;
	std::size_t						outgoing_degree() const;
	std::size_t						incoming_degree() const;

	bool							edge_contains(edge_dir_t flag, node_id_t dest) const;
	bool							edge_outgoing_contains(node_id_t dest) const;
	bool							edge_incoming_contains(node_id_t dest) const;

	std::span<node_dir_id_t>		edges_span();
	std::span<const node_dir_id_t>	edges_span() const;
	auto							edges_outgoing_range(); //range outgoing edge
	const auto						edges_outgoing_range() const; //const range outgoing edge
	const auto						edges_outgoing_range_sized() const;//const sized range outgoing edge

	bool						insert_edge(node_id_t dest);
	bool						remove_edge(node_id_t dest);
	void						sort_edge();
};

struct dyn_dir_graph{
	using node_t				= dyn_dir_node;
	using node_map_t			= std::unordered_map<
		node_id_t,	//key
		dyn_dir_node,	//value
		std::hash<node_id_t>,
		std::equal_to<node_id_t>,
		std::allocator<std::pair<const node_id_t, dyn_dir_node>>
	>;
	using node_map_it_t			= node_map_t::iterator;
	using node_map_cit_t		= node_map_t::const_iterator;

	using node_map_pair_range_t	= std::ranges::subrange<node_map_t::iterator>;
	using node_map_pair_crange_t= std::ranges::subrange<node_map_t::const_iterator>;
	using node_map_pair_srange_t= std::ranges::subrange<
		node_map_t::const_iterator, node_map_t::const_iterator,
		std::ranges::subrange_kind::sized>;	//sized range, const by design

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

// dyn_dir_node start here

bool dyn_dir_node::edge_contains(node_id_t dest) const{
}

std::span<node_id_t> dyn_dir_node::edges_span(){
}

std::span<const node_id_t> dyn_dir_node::edges_span() const{
}

bool dyn_dir_node::insert_edge(node_id_t dest){
}

bool dyn_dir_node::remove_edge(node_id_t dest){
}

void dyn_dir_node::sort_edge(){
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

dyn_dir_graph::node_range_t dyn_dir_graph::node_range(){
	return std::ranges::subrange{_node_map.begin(), _node_map.end()} | std::views::values;
}

dyn_dir_graph::node_crange_t dyn_dir_graph::node_range() const{
	return std::ranges::subrange{_node_map.cbegin(), _node_map.cend()} | std::views::values;
}

dyn_dir_graph::node_srange_t dyn_dir_graph::node_range_sized() const{
	return std::ranges::subrange{_node_map.cbegin(), _node_map.cend(), _node_map.size()} | std::views::values;
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
std::hash<Small_Graph::dynamic_graph::dyn_dir_node>::operator()
(const Small_Graph::dynamic_graph::dyn_dir_node& node) const noexcept{
	return std::hash<Small_Graph::dynamic_graph::dyn_dir_node::edge_vec_t>{}();
}