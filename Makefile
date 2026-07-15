# Makefile

.PHONY: gcc_build gcc_make clang_build clang_make gcc_run clang_run build run clean rebuild

gcc_build:
	CXX=g++-16 cmake -G Ninja -B build_gcc

gcc_make:
	cmake --build build_gcc

clang_build:
	ln -sf /usr/lib/llvm-20/share/libc++/v1 .external_modules
	CXX=clang++-20 cmake -G Ninja -B build_clang
	ln -sf build_clang/compile_commands.json .

clang_make:
	cmake --build build_clang

gcc_run: gcc_make
	@echo "\n--- Running GCC Build ---"
	./build_gcc/main
    
clang_run: clang_make
	@echo "\n--- Running Clang Build ---"
	perf stat -d ./build_clang/main

clean:
	rm -rf ./build* ./*.json
	rm -rf .external_modules

# ---------------------------------------------------
# UPDATED: These now trigger both compilers!
# ---------------------------------------------------
build: gcc_build clang_build

rebuild: clean build

run: gcc_make clang_make gcc_run clang_run