# DFS & BFS Search Profiling Report 02

## 1. Analysis Context
*Focused on analyzing the `SearchVisitor` benchmarking block inside `dfs_loop`.*
```cpp
for(int i = 0; i < 10000; ++i){
    g.dfs_loop(4872, SearchVisitor{});
}
```

## 2. Performance Profiling
We ran `perf stat` checking exclusively the 10,000 iteration search test:
- **Execution Time:** `~61ms` (Clang) / `~93ms` (GCC).
- **Instruction count:** Still high relative to algorithmic expectation.

We dumped the raw assembly using `objdump -d` focusing on the generated loop inside `main` representing `dfs_loop`. 

**Extracted ASM Hotspots:**
```asm
66af: call   3710 <_Znwm@plt>       # operator new(unsigned long)
66bf: call   3630 <memset@plt>      # Zeroes memory
...
66d2: call   3710 <_Znwm@plt>       # operator new(unsigned long)
...
67a6: call   3730 <_ZdlPvm@plt>     # operator delete(void*, unsigned long)
67b9: call   3730 <_ZdlPvm@plt>     # operator delete(void*, unsigned long)
```

## 3. The Bottleneck: Heap Allocation Inside the Loop
The primary performance bottleneck is specifically caused by **dynamic memory allocation and zeroing** at the beginning of every `dfs_loop` function invocation.

### The Code Reason
Inside `Graph_csr.cppm`, the `dfs_loop` natively declares its working memory internally:
```cpp
std::vector<node> stack;
std::vector<std::uint64_t> visit_vec;
visit_vec.resize((_node_vec.size() >> 6) + 1, 0);
stack.reserve(_node_vec.size() >> 1);
```

When you loop `dfs_loop` 10,000 times, the code instructs the Operating System to dynamically locate heap memory `.reserve()` and initialize it `.resize(..., 0)` using `memset`:
1. `_Znwm` (`operator new`) is called twice per loop (once for `stack`, once for `visit_vec`). `10,000 * 2 = 20,000` allocations.
2. `memset` is called 10,000 times to set all the `uint64_t` bits to zero.
3. `_ZdlPvm` (`operator delete`) is called twice per loop to destroy `stack` and `visit_vec` when the method scope ends upon discovering the node.

Since `SearchVisitor` breaks out completely traversing only a handful of paths early on, the **Memory Allocation takes far longer than the actual graph traversal computation**.

## 4. Proposed Mitigation
To completely unleash traversal speeds for high-throughput searches, **Algorithm Contexts should be hoisted**.

Instead of allocating locally, the method should be refactored to accept a reusable traversal context (a struct retaining `stack` and `visit_vec`).
Between iterations, you simply `.clear()` the stack (which avoids `delete`/`new` and resets the size to `0`), and use `std::fill` or `memset` to only zero out the memory boundaries previously touched, practically eliminating heap management latency entirely!
