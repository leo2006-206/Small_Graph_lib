import std;
import Helper;
import Graph;

std::uint32_t encoding(std::uint32_t target, std::span<std::uint8_t, 5> buff){
	std::uint32_t idx = 0;
	do{
		std::uint8_t content = target & 0x7f;
		target = target >> 7;

		// std::println("content = {}, target = {}, idx = {}", content, target, idx);
		if(target != 0){
			buff[idx] = 0x80 | content;
		}
		else{
			buff[idx] = content;
		}
		idx += 1;
	}while(target != 0);
	// std::println("result = {}", buff);
	return idx;
}

std::uint32_t decode(std::span<std::uint8_t, 5> buff){
	std::uint32_t result{}, idx = 0;
	for(auto target : buff){
		std::uint32_t content = target & (0x7f);
		result |= (content << idx);
		idx += 7;

		if((target & 0x80) == 0){
			break;
		}
	}

	return result;
};

void fun1(){
	std::array<std::uint8_t, 5> buff;

	std::mt19937 mt(123);
	std::uniform_int_distribution<std::uint32_t> dis;

	while(1){
		std::uint32_t item = dis(mt);
		encoding(item, buff);

		if(decode(buff) != item){
			std::println("error item = {}", item);
			break;
		}
	}
}

int main(void){
	Debug::clear_log();

	fun1();

	return 0;
}