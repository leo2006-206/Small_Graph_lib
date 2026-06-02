import std;

template<bool b_v>
struct s{
    int i{123};

    // Trailing requires clause goes AFTER the parameters
    int rt(int a, int b) requires (b_v == true) {
        return i + a + b;
    }

    // Notice the parentheses around the negation!
    int rt(int a) requires (b_v == false) {
        return i + a;
    }
};

int main(void){
    s<true> mys;
    std::println("{}", mys.rt(1, 2));

    s<false> mys2;
    std::println("{}", mys2.rt(1));
}