import std;

struct empty_t{
	explicit empty_t() = default;
};

template<bool b_v>
struct s{
    int i{123}; // Default initialization here is clean
    [[no_unique_address]] std::conditional_t<b_v, float, empty_t> extra;

    // Constructor for the true case (initializes the float)
    s() requires (b_v == true) : extra{15.15f} {}

    // Constructor for the false case (does nothing extra)
    s() requires (b_v == false) {}

    int rt(int a, int b) requires (b_v == true) {
        return i + a + b;
    }

    int rt(int a) requires (b_v == false) {
        return i + a;
    }
};
int main(void){
    s<true> mys;
    std::println("{}, f {}, sizeof {}", mys.rt(1, 2), mys.extra, sizeof(decltype(mys)));

    s<false> mys2;
    std::println("{} sizeof {}", mys2.rt(1), sizeof(decltype(mys2)));
}