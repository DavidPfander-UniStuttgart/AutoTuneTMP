cmake_minimum_required(VERSION 3.5)

project(AutoTuneTMP)
set(AutoTuneTMP_VERSION 0.1.0)

add_library(AutoTuneTMP INTERFACE)

find_package(Vc ${Vc_FIND_VERSION} QUIET NO_MODULE PATHS ${Vc_ROOT})

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

message("Vc_INCLUDE_DIR: " ${Vc_INCLUDE_DIR})
# message("BOOST_ROOT: " ${BOOST_ROOT})
# message("Boost_FOUND: " ${Boost_FOUND})
# message("Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})

# target_include_directories(AutoTuneTMP INTERFACE ${Vc_INCLUDE_DIR})
target_include_directories(AutoTuneTMP INTERFACE
  $<BUILD_INTERFACE:${Vc_INCLUDE_DIR}>
  )

find_package(cppjit NO_MODULE REQUIRED PATHS ${CPPJIT_ROOT})
target_link_libraries(AutoTuneTMP INTERFACE cppjit::cppjit)

target_include_directories(AutoTuneTMP INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/>
  )

install(TARGETS AutoTuneTMP EXPORT AutoTuneTMPTargets)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/AutoTuneTMPConfigVersion.cmake"
  VERSION ${AutoTuneTMP_VERSION}
  COMPATIBILITY AnyNewerVersion
  )

export(EXPORT AutoTuneTMPTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/AutoTuneTMPTargets.cmake"
  NAMESPACE AutoTuneTMP::
  )

configure_file(AutoTuneTMPConfig.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/AutoTuneTMPConfig.cmake"
  COPYONLY
  )
set(ConfigPackageLocation lib/cmake/AutoTuneTMP)

install(EXPORT AutoTuneTMPTargets
  FILE
  AutoTuneTMPTargets.cmake
  NAMESPACE
  AutoTuneTMP::
  DESTINATION
  ${ConfigPackageLocation}
  )


install(
  FILES
  AutoTuneTMPConfig.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/AutoTuneTMPConfigVersion.cmake"
  DESTINATION
  ${ConfigPackageLocation}
  COMPONENT
  Devel
  )

# install headers and preserve directory structure
INSTALL (
  DIRECTORY ${CMAKE_SOURCE_DIR}/include/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h*")

# set(CMAKE_CXX_FLAGS "-march=native -mno-vzeroupper -ffast-math -std=c++17 -g -Wall -Wextra -Wno-unused-parameter ${CMAKE_CXX_FLAGS}")

# -Wconversion
target_compile_options(AutoTuneTMP INTERFACE -std=c++17 -march=native -mtune=native -mno-vzeroupper -ffast-math -Wall -Wextra -Wcast-qual -Wformat=2 -Wformat-nonliteral -Wformat-security -Winit-self -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked -Wunreachable-code -Wunused -Wno-unused-parameter -Wswitch-enum -Wredundant-decls -pedantic -Wswitch-default ${CMAKE_CXX_FLAGS})

#################
# build examples
#################

set(SOURCES_UNROLL_LOOP "examples/unroll_example.cpp")
add_executable(unroll_example ${SOURCES_UNROLL_LOOP})
target_link_libraries(unroll_example AutoTuneTMP)
install(TARGETS unroll_example DESTINATION examples)

add_executable(loop_nest_example "examples/loop_nest_example.cpp")
target_link_libraries(loop_nest_example AutoTuneTMP)
install(TARGETS loop_nest_example DESTINATION examples)

add_executable(unroll_example_autotune_inline "examples/unroll_loop_autotune_inline.cpp")
target_link_libraries(unroll_example_autotune_inline AutoTuneTMP)
install(TARGETS unroll_example_autotune_inline DESTINATION examples)

add_executable(unroll_example_autotune "examples/unroll_loop_autotune.cpp")
target_link_libraries(unroll_example_autotune AutoTuneTMP)
install(TARGETS unroll_example_autotune DESTINATION examples)

add_executable(unroll_example_autotune_return_int "examples/unroll_loop_autotune_return_int.cpp")
target_link_libraries(unroll_example_autotune_return_int AutoTuneTMP)
install(TARGETS unroll_example_autotune_return_int DESTINATION examples)

add_executable(minimal_example "examples/minimal_example.cpp")
target_link_libraries(minimal_example AutoTuneTMP)
install(TARGETS minimal_example DESTINATION examples)

add_executable(jit_only_kernel "examples/jit_only_example.cpp")
target_link_libraries(jit_only_kernel AutoTuneTMP AutoTuneTMP Boost::boost)
install(TARGETS jit_only_kernel DESTINATION examples)

add_executable(simplified_jit_only_kernel "examples/simplified_jit_only_example.cpp")
target_link_libraries(simplified_jit_only_kernel AutoTuneTMP AutoTuneTMP Boost::boost)
install(TARGETS simplified_jit_only_kernel DESTINATION examples)

add_executable(matrix_vector_blocking "examples/matrix_vector_blocking.cpp")
target_compile_options(matrix_vector_blocking PUBLIC -fopenmp)
target_link_libraries(matrix_vector_blocking AutoTuneTMP AutoTuneTMP Boost::boost -fopenmp)
install(TARGETS matrix_vector_blocking DESTINATION examples)

add_executable(simplified_matrix_vector_blocking "examples/simplified_matrix_vector_blocking.cpp")
target_compile_options(simplified_matrix_vector_blocking PUBLIC -fopenmp)
target_link_libraries(simplified_matrix_vector_blocking AutoTuneTMP AutoTuneTMP Boost::boost -fopenmp)
install(TARGETS simplified_matrix_vector_blocking DESTINATION examples)

add_executable(matrix_tiling "examples/matrix_tiling.cpp")
target_link_libraries(matrix_tiling AutoTuneTMP AutoTuneTMP Boost::boost)
install(TARGETS matrix_tiling DESTINATION examples)

add_executable(matrix_tiling_iterate "examples/matrix_tiling_iterate.cpp")
target_compile_options(matrix_tiling_iterate PUBLIC -fopenmp)
target_link_libraries(matrix_tiling_iterate AutoTuneTMP AutoTuneTMP Boost::boost -fopenmp)
install(TARGETS matrix_tiling_iterate DESTINATION examples)

add_executable(continuous_parameter_example "examples/continuous_parameter_example.cpp")
target_link_libraries(continuous_parameter_example AutoTuneTMP)
install(TARGETS continuous_parameter_example DESTINATION examples)

add_executable(bruteforce_search_example "examples/bruteforce_search_example.cpp")
target_link_libraries(bruteforce_search_example AutoTuneTMP)
install(TARGETS bruteforce_search_example DESTINATION examples)

add_executable(parallel_line_search_example "examples/parallel_line_search_example.cpp")
target_link_libraries(parallel_line_search_example AutoTuneTMP)
install(TARGETS parallel_line_search_example DESTINATION examples)

add_executable(countable_set_example "examples/countable_set_example.cpp")
target_link_libraries(countable_set_example AutoTuneTMP)
install(TARGETS countable_set_example DESTINATION examples)

add_executable(simple_sor_diffusion "examples/simple_sor_diffusion.cpp")
target_link_libraries(simple_sor_diffusion AutoTuneTMP)
install(TARGETS simple_sor_diffusion DESTINATION examples)

add_executable(neighborhood_search_example "examples/neighborhood_search_example.cpp")
target_link_libraries(neighborhood_search_example AutoTuneTMP)
install(TARGETS neighborhood_search_example DESTINATION examples)

add_executable(greedy_neighborhood_search_example "examples/greedy_neighborhood_search_example.cpp")
target_link_libraries(greedy_neighborhood_search_example AutoTuneTMP)
install(TARGETS greedy_neighborhood_search_example DESTINATION examples)

add_executable(monte_carlo_example "examples/monte_carlo_example.cpp")
target_link_libraries(monte_carlo_example AutoTuneTMP)
install(TARGETS monte_carlo_example DESTINATION examples)

add_executable(register_blocking_example "examples/register_blocking.cpp")
target_link_libraries(register_blocking_example AutoTuneTMP)
install(TARGETS register_blocking_example DESTINATION examples)

add_executable(register_blocking_minimal_example "examples/register_blocking_minimal.cpp")
# target_compile_options(register_blocking_minimal_example PUBLIC AutoTuneTMP Boost::boost)
target_include_directories(register_blocking_minimal_example PUBLIC ${Boost_INCLUDE_DIRS})
# target_include_directories(register_blocking_minimal_example PUBLIC Boost::boost)
target_link_libraries(register_blocking_minimal_example AutoTuneTMP)
install(TARGETS register_blocking_minimal_example DESTINATION examples)

add_executable(mult_example "examples/mult_example.cpp")
target_link_libraries(mult_example AutoTuneTMP)
target_compile_options(mult_example PUBLIC -fopenmp)
install(TARGETS mult_example DESTINATION examples)

add_executable(group_tuner_example "examples/group_tuner_example.cpp")
target_link_libraries(group_tuner_example AutoTuneTMP)
target_compile_options(group_tuner_example PUBLIC -fopenmp)
install(TARGETS group_tuner_example DESTINATION examples)

add_executable(group_tuner_dep_example "examples/group_tuner_dep_example.cpp")
target_link_libraries(group_tuner_dep_example AutoTuneTMP)
target_compile_options(group_tuner_dep_example PUBLIC -fopenmp)
install(TARGETS group_tuner_dep_example DESTINATION examples)

add_executable(group_tuner_simple_dep_example "examples/group_tuner_simple_dep_example.cpp")
target_link_libraries(group_tuner_simple_dep_example AutoTuneTMP)
target_compile_options(group_tuner_simple_dep_example PUBLIC -fopenmp)
install(TARGETS group_tuner_simple_dep_example DESTINATION examples)

add_executable(thread_pool_example "examples/thread_pool_example.cpp")
target_link_libraries(thread_pool_example AutoTuneTMP -pthread)
target_compile_options(thread_pool_example PUBLIC -fopenmp)
install(TARGETS thread_pool_example DESTINATION examples)

add_executable(queue_thread_pool_example "examples/queue_thread_pool_example.cpp")
target_link_libraries(queue_thread_pool_example AutoTuneTMP -pthread)
target_compile_options(queue_thread_pool_example PUBLIC -fopenmp)
install(TARGETS queue_thread_pool_example DESTINATION examples)

add_executable(grid_executor_example "examples/grid_executor_example.cpp")
target_link_libraries(grid_executor_example AutoTuneTMP -pthread)
target_compile_options(grid_executor_example PUBLIC -fopenmp)
install(TARGETS grid_executor_example DESTINATION examples)

add_executable(tuned_grid_executor_example "examples/tuned_grid_executor_example.cpp")
target_link_libraries(tuned_grid_executor_example AutoTuneTMP -pthread)
target_compile_options(tuned_grid_executor_example PUBLIC -fopenmp)
install(TARGETS tuned_grid_executor_example DESTINATION examples)

# add_executable(tuned_grid_executor_generalized_kernel_example "examples/tuned_grid_executor_generalized_kernel_example.cpp")
# target_link_libraries(tuned_grid_executor_generalized_kernel_example AutoTuneTMP -pthread)
# target_compile_options(tuned_grid_executor_generalized_kernel_example PUBLIC -fopenmp)
# install(TARGETS tuned_grid_executor_generalized_kernel_example DESTINATION examples)

add_executable(copyable_kernel_example "examples/copyable_kernel_example.cpp")
target_link_libraries(copyable_kernel_example AutoTuneTMP -pthread)
target_compile_options(copyable_kernel_example PUBLIC -fopenmp)
install(TARGETS copyable_kernel_example DESTINATION examples)

add_executable(boost_tests "tests/boost_tests.cpp")
target_link_libraries(boost_tests PUBLIC AutoTuneTMP Boost::boost Boost::unit_test_framework)

#################
# end examples
#################

#################
# paper examples
#################

# set(SOURCES_FMM_M2M_INTERACTIONS
#   "paper/fmm_m2m_interactions/fmm_m2m_interactions.cpp"
#   "paper/fmm_m2m_interactions/kernels/m2m_interactions.cpp"
#   "paper/fmm_m2m_interactions/kernels/m2m_kernel.cpp"
#   "paper/fmm_m2m_interactions/kernels/m2m_kernel.cpp"
#   "paper/fmm_m2m_interactions/kernels/m2m_kernel_blocked_interaction.cpp"
#   "paper/fmm_m2m_interactions/kernels/calculate_stencil.cpp"
#   "paper/fmm_m2m_interactions/compute_factor.cpp"
#   "paper/fmm_m2m_interactions/taylor.cpp"
#   "paper/fmm_m2m_interactions/compute_ilist.cpp"
#   "paper/fmm_m2m_interactions/new.cpp"
#   "paper/fmm_m2m_interactions/kernel_reference.cpp"
# )
# add_executable(fmm_m2m_interactions ${SOURCES_FMM_M2M_INTERACTIONS})
# install(TARGETS fmm_m2m_interactions DESTINATION paper)

#################
# end paper examples
#################
