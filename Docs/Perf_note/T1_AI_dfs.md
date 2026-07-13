`Monday, July 13, 2026 in Manchester, UK`

This analysis treats your empirical logs as a comparative microarchitectural study. The data reveals a fascinating story: **graph topology and structural indexing matter far more to hardware execution efficiency than raw graph size.**

---

## 1. Consolidated Performance Metrics Matrix

The table below aggregates the raw hardware counters, Top-down Microarchitecture Analysis (TMA) metrics, and graph structural properties extracted from your experiments.

| Metric / Feature | Amazon Product 2003 | Pokec Social | UK 2002 |
| --- | --- | --- | --- |
| **Graph Topology Type** | Co-purchase Network | Social Network | Web Crawl / Link Graph |
| **Total CSR Size** | 16.78 MB | 135.55 MB | 1.32 GB |
| **Memory Jump Mean ($\mu$)** | 1,798,262 | 22,944,997 | 14,088,376 |
| **Memory Jump Std ($\sigma$)** | 2,228,549 | 27,290,898 | 90,855,209 |
| **L1-D Cache Miss Rate** | 13.14% | 27.95% | **1.49%** |
| **LLC Miss Rate** | **20.80%** | 53.80% | 63.74% |
| **Instructions Per Cycle (IPC)** | 0.76 | 0.68 | **1.79** |
| **TMA: Backend Bound** | 58.8% | **69.6%** | 44.7% |
| **TMA: Bad Speculation** | 21.6% | 15.0% | 19.4% |
| **TMA: Frontend Bound** | 7.5% | 4.9% | 8.2% |
| **TMA: Retiring** | 12.1% | 10.5% | **27.6%** |

---

## 2. Deep-Dive Performance Analysis

### A. Memory Size vs. Cache Miss Hierarchy

* **The L3 Cache Threshold (`amazon_product_2003`):** At a compact **16.78 MB**, the entire Amazon graph fits comfortably inside the Last Level Cache (LLC) of modern performance-core CPUs (which typically feature 16MB to 96MB of L3). Consequently, its LLC miss rate is exceptionally low (**20.80%**), meaning very few traversals are forced to hit slow system DRAM.
* **The DRAM Penalty (`pokec_social` & `uk_2002`):** At **135.55 MB** and **1.32 GB**, these graphs completely shatter the capacity of hardware caches. When a thread misses L1 and L2, it frequently misses the LLC as well (**53.80%** and **63.74%** respectively), forcing the CPU execution pipeline to stall while waiting for high-latency main memory fetches.

### B. The Jump Metric Paradox: Spatial Locality vs. Structural Chaos

The relationship between the memory address "jump" statistics (the distance between node $i$ and node $i+1$ in traversal) and the L1 Data Cache behavior highlights the differences in graph structure:

* **Social Graph Randomness (`pokec_social`):** Social networks exhibit poor spatial locality because edge connections span globally across the dataset without a strict linear cluster layout. The high jump mean (**22.9M**) combined with a high standard deviation (**27.2M**) means the CPU is constantly jumping to distant, un-cached memory addresses. This inflicts a massive **27.95% L1-D miss rate**, thrashing the closest hardware caches.
* **Web Graph Locality Clustering (`uk_2002`):** This is the most compelling result. Despite `uk_2002` being the largest graph by far (1.32 GB) and possessing a massive jump standard deviation (**90.8M**), it registers a microscopic **1.49% L1-D miss rate**.
This occurs because web graphs crawled by hostnames/URLs inherently cluster. The massive standard deviation indicates large, macro-level jumps when crossing domains, but the vast majority of local traversals occur within tightly localized pages on the same host. The L1 cache captures this local structural reuse perfectly, keeping the data near the execution engine.

### C. Microarchitectural Impact (TMA & IPC Execution)

```
   Graph Locality & Size Dynamics
   ----------------------------------------------------------------------------
   [pokec_social] ---> High L1 Miss (27.95%) + High LLC Miss (53.80%) 
                       └──> Stalls Execution ──> 69.6% Backend Bound ──> IPC 0.68
   
   [uk_2002]      ---> Low L1 Miss (1.49%)  + High LLC Miss (63.74%)
                       └──> Smooth L1 Pipeline ──> 27.6% Retiring    ──> IPC 1.79
   ----------------------------------------------------------------------------

```

* **Why `pokec_social` Chokes the Pipeline:** Because it suffers from high miss rates at *both* the L1-D level and the LLC level, the core spends most of its time waiting for data to arrive from the memory controller. This creates a severe memory stall condition, manifesting as the highest **Backend Bound** metric (**69.6%**) and a poor **IPC of 0.68**.
* **Why `uk_2002` Achieves High Throughput:** With an L1-D hit rate of over **98.5%**, the CPU execution units stay heavily fed with local data. Even though it suffers from severe LLC misses when it *does* break out of a cluster, the out-of-order execution engine successfully hides much of that latency behind successful L1 hits. This shifts the microarchitecture toward productive work, yielding the highest **Retiring** rate (**27.6%**) and a highly efficient **IPC of 1.79**.

---

## 3. Key Architectural Takeaways

1. **Don't Fear Large Graphs if They Have High Locality:** A massive graph like `uk_2002` can achieve **2.6× the execution efficiency (IPC)** of a smaller graph like `pokec_social` if its indexing naturalizes spatial locality.
2. **The Backend Bottleneck:** For graph processing, `tma_backend_bound` is directly tied to the *intersection* of L1-D and LLC miss rates. If you can keep L1-D miss rates low via clever node reordering (like BFS/Cuthill-McKee orderings), you can mitigate the architectural damage caused by a graph that is too large for the LLC.

Would you like to explore node reordering techniques (like HubSorting or Rabbit Orders) to see how dramatically we can compress the jump metrics and L1-D miss rates for the chaotic social graph?