import std;

auto main(void) -> int{
	std::vector<int> v;
	v.reserve(20);

	std::print("\n c = {}", v.capacity());

	std::priority_queue<int, std::vector<int>, std::greater<int>> q(std::greater<int>(), std::move(v));
	q.emplace(10);
	q.emplace(30);
	q.emplace(20);
	q.emplace(40);

	std::print("\n q = {}", q);


}