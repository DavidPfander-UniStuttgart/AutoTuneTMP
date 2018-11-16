#pragma once

#include <likwid.h>
#include <memory>
#include <sched.h>

namespace opttmp {
class numa_topology_t {
private:
  uint32_t threads_total;
  uint32_t threads_socket;
  uint32_t sockets;
  std::vector<std::vector<uint32_t>> socket_threads_map;

  static void delete_cpu_set(cpu_set_t *cpu_set) { CPU_FREE(cpu_set); }

public:
  numa_topology_t() {
    int err = topology_init();
    if (err < 0) {
      printf("Failed to initialize LIKWID's topology module\n");
      throw;
    }

    // CpuInfo_t contains global information like name, CPU family, ...
    // CpuInfo_t info = get_cpuInfo();
    // CpuTopology_t contains information about the topology of the CPUs.
    CpuTopology_t topo = get_cpuTopology();
    threads_total = topo->numHWThreads;

    err = numa_init();
    if (err < 0) {
      printf("Failed to initialize LIKWID's numa module\n");
      throw;
    }
    NumaTopology_t numa_topo = get_numaTopology();
    // std::cout << "numa nodes: " << numa_topo->numberOfNodes << std::endl;
    // assume that each number node contains the same number of processors
    threads_socket = numa_topo->nodes[0].numberOfProcessors;
    // std::vector<std::vector<uint32_t>> sorted_numa_nodes(
    //     numa_topo->numberOfNodes);
    sockets = numa_topo->numberOfNodes;
    socket_threads_map.resize(sockets);
    for (size_t i = 0; i < sockets; i += 1) {
      socket_threads_map[i].resize(threads_socket);
    }
    for (size_t i = 0; i < sockets; i += 1) {
      NumaNode node = numa_topo->nodes[i];
      for (size_t j = 0; j < node.numberOfProcessors; j += 1) {
        socket_threads_map[i][j] = node.processors[j];
      }
      std::sort(socket_threads_map[i].begin(), socket_threads_map[i].end());
    }

    numa_finalize();
    topology_finalize();
  }
  uint32_t get_threads_total() { return threads_total; }
  uint32_t get_threads_socket() { return threads_socket; }
  uint32_t get_sockets() { return sockets; }
  uint32_t operator()(uint32_t socket, uint32_t cpu_num) {
    return socket_threads_map[socket][cpu_num];
  }

  void print() {
    numa_topology_t &numa_topology = *this;
    for (size_t i = 0; i < numa_topology.get_sockets(); i += 1) {
      std::cout << "numa node: " << i << ": ";
      for (size_t j = 0; j < numa_topology.get_threads_socket(); j += 1) {
        if (j > 0)
          std::cout << ", ";
        // std::cout << node.processors[j];
        std::cout << numa_topology(i, j);
      }
      std::cout << std::endl;
    }
  }

  std::shared_ptr<cpu_set_t> get_cpuset_compact(uint32_t num_threads) {
    numa_topology_t &numa_topology = *this;
    std::shared_ptr<cpu_set_t> cpu_set(CPU_ALLOC(threads_total),
                                       delete_cpu_set);
    size_t size_bytes = CPU_ALLOC_SIZE(threads_total);
    CPU_ZERO_S(size_bytes, cpu_set.get());
    // CPU_ZERO_S(size, cpu_set.get()o);
    size_t cpus_assigned = 0;
    for (size_t i = 0; i < sockets; i += 1) {
      for (size_t j = 0; j < threads_socket; j += 1) {
        CPU_SET(numa_topology(i, j), cpu_set);
        cpus_assigned += 1;
        if (cpus_assigned >= num_threads) {
          break;
        }
      }
      if (cpus_assigned >= num_threads) {
        break;
      }
    }

    return cpu_set;
  }

  std::shared_ptr<cpu_set_t> get_cpuset_sparse(uint32_t num_threads) {
    numa_topology_t &numa_topology = *this;
    std::shared_ptr<cpu_set_t> cpu_set(CPU_ALLOC(threads_total),
                                       delete_cpu_set);
    size_t size_bytes = CPU_ALLOC_SIZE(threads_total);
    CPU_ZERO_S(size_bytes, cpu_set.get());
    size_t cpus_assigned = 0;
    for (size_t j = 0; j < threads_total; j += 1) {
      for (size_t i = 0; i < sockets; i += 1) {
        CPU_SET(numa_topology(i, j), cpu_set);
        cpus_assigned += 1;
        if (cpus_assigned >= num_threads) {
          break;
        }
      }
      if (cpus_assigned >= num_threads) {
        break;
      }
    }

    return cpu_set;
  }
  std::shared_ptr<cpu_set_t> get_cpuset_full() {
    numa_topology_t &numa_topology = *this;
    std::shared_ptr<cpu_set_t> cpu_set(CPU_ALLOC(threads_total),
                                       delete_cpu_set);
    size_t size_bytes = CPU_ALLOC_SIZE(threads_total);
    CPU_ZERO_S(size_bytes, cpu_set.get());
    for (size_t j = 0; j < threads_total; j += 1) {
      for (size_t i = 0; i < sockets; i += 1) {
        CPU_SET(numa_topology(i, j), cpu_set);
      }
    }
    return cpu_set;
  }

  std::vector<bool> get_compact(uint32_t num_threads) {
    numa_topology_t &numa_topology = *this;
    std::vector<bool> cpu_set(threads_total, false);
    size_t cpus_assigned = 0;
    for (size_t i = 0; i < sockets; i += 1) {
      for (size_t j = 0; j < threads_socket; j += 1) {
        cpu_set[numa_topology(i, j)] = true;
        cpus_assigned += 1;
        if (cpus_assigned >= num_threads) {
          break;
        }
      }
      if (cpus_assigned >= num_threads) {
        break;
      }
    }
    return cpu_set;
  }

  std::vector<bool> get_sparse(uint32_t num_threads) {
    numa_topology_t &numa_topology = *this;
    std::vector<bool> cpu_set(threads_total, false);
    size_t cpus_assigned = 0;
    for (size_t i = 0; i < sockets; i += 1) {
      for (size_t j = 0; j < threads_socket; j += 1) {
        cpu_set[numa_topology(i, j)] = true;
        cpus_assigned += 1;
        if (cpus_assigned >= num_threads) {
          break;
        }
      }
      if (cpus_assigned >= num_threads) {
        break;
      }
    }
    return cpu_set;
  }

  std::vector<bool> get_full() {
    // numa_topology_t &numa_topology = *this;
    std::vector<bool> cpu_set(threads_total, true);
    return cpu_set;
  }
  std::vector<bool> get_empty() {
    // numa_topology_t &numa_topology = *this;
    std::vector<bool> cpu_set(threads_total, false);
    return cpu_set;
  }

  void print_cpu_set(std::shared_ptr<cpu_set_t> cpu_set) {
    for (size_t i = 0; i < threads_total; i += 1) {
      if (CPU_ISSET(i, cpu_set.get())) {
        std::cout << "1";
      } else {
        std::cout << "0";
      }
    }
    std::cout << std::endl;
  }
};
} // namespace opttmp
