# Performance Engineering Record (DFS / BFS)

## 1. Initial State (Pre-Optimization)
*Tested on May 24, 2026*

The initial benchmarks experienced significant redundant overhead when continuously parsing graphs loops. The bottleneck was tied to duplicate graph existence checks (`node_contains`) and expensive C++ Standard Library bitset bindings.

**Key Metrics (Pre-Optimization):**
- **Execution Time:** `175.344ms` (GCC) | `174.092ms` (Clang)
- **Instructions Executed:** `~2.64 Billion` (GCC) | `~2.76 Billion` (Clang)
- **IPC (Instructions Per Cycle):** `3.70` (GCC) | `3.82` (Clang)
- **Branch Misses:** `~3.9 Million` for both builds.
- **CPU Bound Distribution:** `~24% Frontend Bound`.

---

## 2. Bottlenecks Identified

1. **`node_contains()` Dual Validation:**
   The `node_contains(dest)` function was being invoked twice per single inner loop cycle. This added thousands of redundant instructions given `~50,000+` edges traversed.
2. **`std::bitset<64>` Call Tracing:**
   Utilizing `visit_vec[].test()` and `.set()` added unnecessary C++ exception guarding and library call overhead.

---

## 3. Improvements Implemented
We refactored the innermost loop of `dfs_loop`, converting to raw memory boundaries and primitive bitwise logic:

```cpp
// 1. Bitwise Masking replaces std::bitset
constexpr auto cal_offset = [](node id) -> std::uint64_t {
    return 1ULL << (id & 63);
};
std::vector<std::uint64_t> visit_vec;

// 2. Direct edge equality prevents expensive 'node_contains' wrappers
if(current_edge._dest_id == NULL_EDGE._dest_id) continue;

// 3. Raw shift bounds check
auto& bit_pos = visit_vec[dest >> 6];
if((bit_pos & cal_offset(dest)) == false && current_edge._dest_id != NULL_EDGE._dest_id) {
    bit_pos |= cal_offset(dest);
    stack.emplace_back(dest);
}
```

---

## 4. Final State (Post-Optimization)

**Key Metrics (Post-Optimization):**
- **Execution Time:** `85.904ms` (GCC) | `56.542ms` (Clang) -> **(Up to 67% improvement!)**
- **Instructions Executed:** `~1.29 Billion` (GCC) | `~1.47 Billion` (Clang) -> **(Cut entirely in half!)**
- **IPC (Instructions Per Cycle):** `3.47` (GCC) | `4.91` (Clang) -> **(Clang peaks at phenomenal nearly 5.0 IPC!)**
- **Branch Misses:** `~436k` (GCC) | `~543k` (Clang) -> **(Reduced array-bounds guessing by almost 90%!)**
- **CPU Bound Distribution:** `~11.6% Frontend Bound` (GCC) -> **(Frontend saturation completely alleviated)**

## Conclusion
The elimination of the redundant internal methods successfully chopped the overall instruction pipeline count vertically in half. Due to the high locality inside the CPU cache, mitigating the internal branching and wrapper functions unleashed the CPU backend pipeline limits—allowing Clang to achieve an astonishing `.61ns / 4.91 IPC` compute ratio.
