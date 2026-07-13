`Monday, July 13, 2026 in Manchester, UK`

Here is a complete microarchitectural performance analysis of your Breadth-First Search (BFS) execution, followed by a direct comparative breakdown against your previous Depth-First Search (DFS) results.

## 1. BFS Consolidated Performance Metrics Matrix

The table below aggregates the hardware counters, Top-down Microarchitecture Analysis (TMA) metrics, and memory jump properties for your BFS runs.

| Metric / Feature | Amazon Product 2003 | Pokec Social | UK 2002 |
| --- | --- | --- | --- |
| **Duration** | 18.057ms| 190.469ms| 803.650ms|
| **Visited Nodes Count** | 401,838| 1,313,194| 15,148,169|
| **Memory Jump Mean ($\mu$)** | 1,438,933| 12,427,482 | 21,284,590 |
| **Memory Jump Std ($\sigma$)** | 2,080,918 | 21,381,035 | 110,539,091 |
| **L1-D Cache Miss Rate** | 11.27% | 25.24% |  1.76% |
| **LLC Miss Rate** | 9.68% | 45.51% | 64.61% |
| **Instructions Per Cycle (IPC)** | 1.08 | 0.79 | 1.58 |
| **TMA: Backend Bound** | 47.5% | 58.9% | 49.4% |
| **TMA: Bad Speculation** | 26.7%| 16.6% | 17.5% |
| **TMA: Retiring** | 17.1% | 15.9% | 24.7% |

---

## 2. BFS Deep-Dive Performance Analysis

### A. The Queue Overhead and Cache Dynamics

In a BFS, the structural frontier is managed by a queue (likely a `std::deque` based on previous diagnostics).

* **The Cache Fit (`amazon_product_2003`):** Because the total CSR graph is only 16.78 MB, the core graph arrays easily fit into the Last Level Cache (LLC). This yields a highly efficient LLC miss rate of just 9.68%. Consequently, the CPU execution pipeline is well-fed, resulting in an IPC of 1.08 and a low Backend Bound metric of 47.5%.


* **The Bottleneck Threshold (`pokec_social` & `uk_2002`):** As the graph sizes scale to 135.55 MB and 1.32 GB, they overwhelm the LLC, creating miss rates of 45.51% and 64.61% respectively. When the queue size explodes during the middle phases of BFS on these massive networks, the CPU must constantly thrash the LLC to maintain the frontier, stalling execution.



### B. Memory Jump Locality in BFS Expansion

* **Social Graph Contraction (`pokec_social`):** The BFS expansion across the social graph forces an average memory jump of 12,427,482. Because BFS processes neighbors in strict tier-order, it is pulling nodes from entirely different geographic or structural clusters simultaneously. This destroys spatial locality, causing a massive 25.24% L1-D cache miss rate.


* **Web Graph Preservation (`uk_2002`):** Strikingly, even with a massive BFS standard deviation of 110,539,091, the web graph maintains an ultra-low L1-D miss rate of just 1.76%. This proves that the local domain clustering of the web graph is so strong that even the wide, level-by-level expansion of BFS cannot entirely defeat the hardware prefetcher and L1 caching for local pages.



---

## 3. Comparative Analysis: BFS vs. DFS

By comparing your BFS execution logs to the previous DFS execution logs, a clear architectural divergence emerges based on graph topology.

### Execution Speed and Throughput (IPC)

* **Small/Random Graphs (Amazon & Pokec):** BFS is significantly faster and more efficient here.
* For `amazon_product_2003`, BFS completed in 18.057ms compared to the DFS time of 22.723ms. BFS achieved a higher IPC (1.08 vs 0.76).


* For `pokec_social`, BFS completed in 190.469ms vs the DFS time of 204.221ms, with a better IPC (0.79 vs 0.68).




* **Massive Clustered Graphs (UK 2002):** DFS completely dominates.
* DFS processed `uk_2002` in 651.183ms, while BFS crawled to 803.650ms. DFS achieved an IPC of 1.79, while BFS dropped to 1.58.





### Why Does DFS Beat BFS on the Web Graph?

The answer lies entirely in the **Memory Jump Mean ($\mu$)**.

When DFS dives deep into a web graph (`uk_2002`), it stays within a highly clustered local domain for a long time before backing out. The DFS memory jump mean was ~14.08 million.

When BFS expands on `uk_2002`, it must process the entire frontier simultaneously. If a homepage links to 50 local pages and 50 external domains, BFS forces the CPU to evaluate all of them at once. This balloons the BFS memory jump mean to 21,284,590. This chaotic, wide frontier expansion increases the L1-D miss rate from 1.49% (DFS) to 1.76% (BFS) and significantly inflates the Bad Speculation to 17.5%, slowing down the total duration.

### Structural vs. Temporal Locality

* **DFS** leverages *temporal locality*. By plunging deep into a cluster, it keeps the recently loaded `node_vector` data hot in the L1 cache. It works exceptionally well for web graphs.
* **BFS** sacrifices temporal locality to evaluate structural tiers. However, on highly random social graphs like `pokec_social`, DFS's deep diving provides no cache benefit because the edges are already random. BFS's queue-based approach actually results in slightly better cache management (LLC misses drop from 53.80% in DFS to 45.51% in BFS), making BFS the superior algorithm for chaotic, unclustered networks.