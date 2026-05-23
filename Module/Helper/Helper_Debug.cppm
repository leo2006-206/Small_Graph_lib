module;

export module Helper:Debug;

import std;

namespace _imp_Debug {

	consteval bool get_DEBUG_STATE(){
		return true;
	}

	std::filesystem::path get_LOG_PATH(){
		return std::filesystem::absolute({"debug_log.log"});
	}

	std::ofstream& get_LOG_FILE(){
		static std::ofstream log_file(_imp_Debug::get_LOG_PATH(), std::ios::app); 
		return log_file;
	}

}

export	namespace Debug{
	template<typename... Args>
	struct	debug_print_log{
		debug_print_log(
			std::format_string<Args...> fmt,
			Args&&... args,
			const std::source_location location = std::source_location::current()
		){
			if constexpr (_imp_Debug::get_DEBUG_STATE()){

				auto source_info = std::format(
					"[INFO, {}, {}, {}]\n",
					location.file_name(),
					location.function_name(),
					location.line()
				);
				
				auto message = std::format(fmt, std::forward<Args>(args) ...);

				static std::mutex _;
				std::lock_guard<std::mutex> _lg(_);

				std::print(std::cout, "\x1B[37m{}\x1B[93m{}\x1B[0m\n\n", source_info, message);
				std::print(_imp_Debug::get_LOG_FILE(), "{}{}\n\n", source_info, message);
			}
		}
	};

	template<typename Fmt, typename... Args>
	debug_print_log(Fmt&&, Args&&...) -> debug_print_log<Args...>;
	

	void	clear_log(){
		std::ofstream ofs(_imp_Debug::get_LOG_PATH(), std::ios::out | std::ios::trunc);
	}
}