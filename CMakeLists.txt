cmake_minimum_required(VERSION 3.5)

project(AutoTuneTMP)
set(AutoTuneTMP_VERSION 0.1.0)

add_library(AutoTuneTMP INTERFACE)

find_package(Vc ${Vc_FIND_VERSION} QUIET NO_MODULE PATHS ${Vc_ROOT})
target_include_directories(AutoTuneTMP INTERFACE ${Vc_INCLUDE_DIR})

find_package(cppjit NO_MODULE REQUIRED PATHS ${CPPJIT_ROOT})
target_link_libraries(AutoTuneTMP INTERFACE cppjit::cppjit)

target_include_directories(AutoTuneTMP INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/>
  )

install(TARGETS AutoTuneTMP EXPORT AutoTuneTMPTargets INCLUDES DESTINATION include)

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

target_compile_options(AutoTuneTMP INTERFACE -std=c++17 ${CMAKE_CXX_FLAGS})

#################
# build examples
#################

set(SOURCES_UNROLL_LOOP "examples/unroll_example.cpp")
add_executable(unroll_example ${SOURCES_UNROLL_LOOP})
target_link_libraries(unroll_example AutoTuneTMP)
install(TARGETS unroll_example DESTINATION examples)

set(SOURCES_LOOP_NEST "examples/loop_nest_example.cpp")
add_executable(loop_nest_example ${SOURCES_LOOP_NEST})
target_link_libraries(loop_nest_example AutoTuneTMP)
install(TARGETS loop_nest_example DESTINATION examples)

set(SOURCES_UNROLL_LOOP_AUTOTUNE_INLINE "examples/unroll_loop_autotune_inline.cpp")
add_executable(unroll_example_autotune_inline ${SOURCES_UNROLL_LOOP_AUTOTUNE_INLINE})
target_link_libraries(unroll_example_autotune_inline dl AutoTuneTMP)
install(TARGETS unroll_example_autotune_inline DESTINATION examples)

set(SOURCES_UNROLL_LOOP_AUTOTUNE "examples/unroll_loop_autotune.cpp")
add_executable(unroll_example_autotune ${SOURCES_UNROLL_LOOP_AUTOTUNE})
target_link_libraries(unroll_example_autotune dl AutoTuneTMP)
install(TARGETS unroll_example_autotune DESTINATION examples)

set(SOURCES_UNROLL_LOOP_AUTOTUNE_RETURN_INT "examples/unroll_loop_autotune_return_int.cpp")
add_executable(unroll_example_autotune_return_int ${SOURCES_UNROLL_LOOP_AUTOTUNE_RETURN_INT})
target_link_libraries(unroll_example_autotune_return_int dl AutoTuneTMP)
install(TARGETS unroll_example_autotune_return_int DESTINATION examples)

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
