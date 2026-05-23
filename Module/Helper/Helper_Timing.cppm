module;

export module Helper:Timing;

import std;

export	namespace Timing{

	std::string	nice_duration_str(
		const std::chrono::nanoseconds duration
	){
		auto ns = duration.count();

		if (ns >= 1'000'000'000) {
			// Greater than 1 billion ns? Print as Seconds (e.g., 1.234 s)
			return std::format("{:.3f}s", std::chrono::duration<double>(duration).count());
			
		} else if (ns >= 1'000'000) {
			// Greater than 1 million ns? Print as Milliseconds (e.g., 45.678 ms)
			return std::format("{:.3f}ms", std::chrono::duration<double, std::milli>(duration).count());
			
		} else if (ns >= 1'000) {
			// Greater than 1 thousand ns? Print as Microseconds (e.g., 890.123 µs)
			return std::format("{:.3f}us", std::chrono::duration<double, std::micro>(duration).count());
			
		} else {
			// Less than 1000 ns? Print as Nanoseconds (no decimals needed here)
			return std::format("{}ns", ns);
		}
	}

	void nice_duration_print(
		const std::chrono::nanoseconds elapsed,
		const char* text = ""
	){
		std::string duration = nice_duration_str(elapsed);

		std::print("\n\x1b[36mDuration {}\t= {}\x1b[0m\n", text, duration);
	}

	struct timer{
		using clock_t_ = std::chrono::steady_clock;

        std::chrono::time_point<clock_t_> 	_before;
	
		timer(): _before(clock_t_::now()){}
	
		void	start(){
			_before = clock_t_::now();
		}
		std::chrono::nanoseconds	end(){
			return clock_t_::now() - _before;
		}
	};

	struct print_timer{
		timer		_timer;
		const char*	_text;

		print_timer(const char* text = "")
		: _timer(), _text(text){}

		~print_timer(){
			nice_duration_print(_timer.end(), _text);
		}
    };

	template <typename F, typename... Args>
    void measure(F&& function, Args&&... argument){
        print_timer _;

        std::invoke(function, argument...);

    }

}