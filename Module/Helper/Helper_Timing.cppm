module;

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>

export module Helper:Timing;

import std;

export	namespace Timing{

	struct perf_control {
		int ctl_fd{-1};

		// Accept the path as a parameter, defaulting to your local fifo
		perf_control(const char* fifo_path = "./perf_control.fifo") {
			// Use O_NONBLOCK so the program doesn't hang if perf isn't running
			ctl_fd = open(fifo_path, O_WRONLY | O_NONBLOCK);
			
			if (ctl_fd == -1) {
				if (errno == ENXIO) {
					// ENXIO specifically means no one is on the other end of the pipe.
					// We don't exit; we just leave ctl_fd as -1 and run normally.
					std::printf("Notice: perf is not listening. Profiling disabled.\n");
				} else {
					// A genuine error (e.g., the FIFO file doesn't exist at all)
					std::perror("Failed to open control FIFO");
					// std::exit(1); 
				}
			}
		}

		~perf_control() {
			if (ctl_fd != -1) {
				close(ctl_fd);
			}
		}

		void start() {
			if (ctl_fd == -1) return; // Do nothing if perf isn't attached

			auto bytes_written = write(ctl_fd, "enable\n", 7);
			if (bytes_written < 0) {
				std::perror("Failed to write 'enable' to control FIFO");
			}
		}

		void finish() {
			if (ctl_fd == -1) return; // Do nothing if perf isn't attached

			auto bytes_written = write(ctl_fd, "disable\n", 8);
			if (bytes_written < 0) {
				std::perror("Failed to write 'disable' to control FIFO");
			}
		}
	};

	struct raii_perf_control{
		perf_control pc;

		raii_perf_control(const char* fifo_path = "./perf_control.fifo")
		: pc(fifo_path){
			pc.start();
		}
		
		~raii_perf_control(){
			pc.finish();
		}
	};

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

		std::print("\n\t\x1b[36mDuration {} = {}\x1b[0m\n", text, duration);
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