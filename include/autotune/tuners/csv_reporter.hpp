#pragma once

#include <cstdlib>

namespace autotune {
class csv_reporter {
private:
  std::string scenario_name;
  std::ofstream scenario_file;
  size_t num_parameters;

  void write_header(parameter_value_set parameter_values) {
    bool first = true;
    for (auto &p : parameter_values) {
      if (!first) {
        scenario_file << ",";
      } else {
        first = false;
      }
      scenario_file << p.first;
    }
    scenario_file
        << ","
        << "kernel_s,kernel_s_raw,compile_s,par_compile_s,par_compile_count"
        << std::endl;
  }

public:
  csv_reporter(const std::string &scenario_name,
               const parameter_value_set &parameter_values,
               const uint64_t tune_counter)
      : scenario_name(scenario_name), num_parameters(parameter_values.size()) {
    std::string hostname(getenv("HOSTNAME"));
    if (hostname.compare("") != 0) {
      scenario_file.open(scenario_name + std::string("_tune_") + hostname +
                         std::string("_") + std::to_string(tune_counter) +
                         std::string("r.csv"));
    } else {
      scenario_file.open(scenario_name + std::string("_tune_nohostname_") +
                         std::to_string(tune_counter) + std::string("r.csv"));
    }
    write_header(parameter_values);
  }

  ~csv_reporter() { scenario_file.close(); }

  void write_measurement(parameter_value_set parameter_values,
                         double duration_kernel_s, double duration_kernel_s_raw,
                         double duration_compile_s) {

    bool first = true;
    for (auto &p : parameter_values) {
      if (!first) {
        scenario_file << ",";
      } else {
        first = false;
      }
      scenario_file << p.second;
    }
    scenario_file << "," << duration_kernel_s << "," << duration_kernel_s_raw
                  << "," << duration_compile_s << "," << 0.0 << "," << 0.0
                  << std::endl;
  }

  void write_compilation(double duration, int64_t no_of_kernels) {
    for (size_t i = 0; i < num_parameters; i += 1) {
      scenario_file << ",";
    }
    scenario_file << 0.0 << "," << 0.0 << "," << 0.0 << "," << duration << ","
                  << no_of_kernels << std::endl;
  }
}; // namespace autotune
} // namespace autotune
