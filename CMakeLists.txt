cmake_minimum_required(VERSION 3.31)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2999bf5c-ef0d-489a-91da-49dbc3090d2a")

project(MyConcurrencyProject CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# ------------------------------------------------------------------------------
# SPEEDUP FIX 1 & 2: Global Compile Options (No hardcoded -O3)
# ------------------------------------------------------------------------------
# These flags will automatically apply to ALL targets created below.
# Options: enforce, observe, ignore, quick-enforce
# Contracts -> -fcontracts -fcontract-evaluation-semantic=enforce
add_compile_options(
	-O3 -march=native
	-Wall -Wextra -Wpedantic -Wconversion
	# -fprofile-generate
	# -fprofile-use
	# -fconcepts-diagnostics-depth=2
	# -fcontracts -fcontract-evaluation-semantic=enforce
)

add_link_options(
	# -fprofile-generate
	# -fprofile-use
)

# ==============================================================================
# 1. THE STANDARD LIBRARY & GLOBAL COMPILER FLAGS
# ==============================================================================
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Configuring GCC 16 Manual Standard Library Module...")
    
    # SPEEDUP FIX 3: Force GCC to use the fast LLVM linker too!
    # add_link_options(-fuse-ld=lld)

    add_library(std_module)
    target_sources(std_module
        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS "/usr/include/c++/16/bits"
        FILES "/usr/include/c++/16/bits/std.cc"
    )
    target_compile_options(std_module PRIVATE -fmodules-ts)

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Configuring Clang 20 Manual Standard Library Module...")
    
    add_compile_options(-stdlib=libc++)
    add_link_options(-stdlib=libc++ -fuse-ld=lld)
    
    add_library(std_module)
    target_sources(std_module
        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/.external_modules"
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/.external_modules/std.cppm"
    )
endif()

# ==============================================================================
# 2. YOUR CUSTOM MODULES
# ==============================================================================

add_library(helper_lib)
target_sources(
	helper_lib PUBLIC 
	FILE_SET CXX_MODULES FILES
		Module/Helper/Helper_Timing.cppm
        Module/Helper/Helper_Debug.cppm
		Module/Helper/Helper.cppm
)
target_link_libraries(helper_lib PRIVATE std_module)

add_library(graph_lib)
target_sources(
	graph_lib PUBLIC 
	FILE_SET CXX_MODULES FILES
		Module/Graph/Graph.cppm
		Module/Graph/Graph_csr.cppm
		Module/Graph/Graph_function.cppm
		Module/Graph/Graph_ds.cppm
		Module/Graph/Graph_dynamic.cppm
		Module/Graph/Graph_ccsr.cppm
)
target_link_libraries(graph_lib PRIVATE std_module helper_lib)

add_library(small_graph_lib)
target_sources(
	small_graph_lib PUBLIC 
	FILE_SET CXX_MODULES FILES
		Module/Small_Graph/SG.cppm
		Module/Small_Graph/SG_dynamic.cppm
		Module/Small_Graph/SG_static_csr.cppm
		Module/Small_Graph/SG_core.cppm
		Module/Small_Graph/SG_algorithm.cppm
		Module/Small_Graph/SG_utility.cppm
)
target_link_libraries(small_graph_lib PRIVATE std_module helper_lib)

# ==============================================================================
# 3. YOUR EXECUTABLES
# ==============================================================================

add_executable(main Driver/main.cpp)
target_link_libraries(main PRIVATE std_module helper_lib graph_lib small_graph_lib)
# SPEEDUP FIX 4: Disabled LTO for daily development
set_property(TARGET main PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

add_executable(temp Driver/temp.cpp)
target_link_libraries(temp PRIVATE std_module)
# SPEEDUP FIX 4: Disabled LTO for daily development
set_property(TARGET temp PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

add_executable(testing Driver/testing.cpp)
target_link_libraries(testing PRIVATE std_module helper_lib graph_lib)
# SPEEDUP FIX 4: Disabled LTO for daily development
set_property(TARGET testing PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

add_executable(testcase Driver/testcase.cpp)
target_link_libraries(testcase PRIVATE std_module helper_lib graph_lib)
# SPEEDUP FIX 4: Disabled LTO for daily development
set_property(TARGET testcase PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)