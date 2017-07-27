cmake_minimum_required(VERSION 3.2)

project(cppjit)

include_directories(include)

set(CMAKE_CXX_FLAGS "-march=native -mno-vzeroupper -ffast-math -std=c++14 -g -Wall -Wextra -Wno-unused-parameter ${CMAKE_CXX_FLAGS}")

include_directories(src)
include_directories(dependencies/cppjit/include)

# add_external(dependencies/Vc "" "")

# ExternalProject_Add(Vc
#   URL https://github.com/VcDevel/Vc
#   PREFIX ""
#   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
#   )

# include(ExternalProject)
# ExternalProject_Add(Vc
#   GIT_REPOSITORY    https://github.com/VcDevel/Vc
#   GIT_TAG           mkretz/datapar
#   SOURCE_DIR        "${CMAKE_BINARY_DIR}/Vc-src"
#   BINARY_DIR        "${CMAKE_BINARY_DIR}/Vc-build"
#   CONFIGURE_COMMAND ""
#   BUILD_COMMAND     ""
#   INSTALL_COMMAND   ""
#   TEST_COMMAND      ""
# )

# add_subdirectory("dependencies/Vc")

find_package(Vc ${Vc_FIND_VERSION} QUIET NO_MODULE PATHS ${Vc_ROOT})
include_directories(${Vc_INCLUDE_DIR})

##################################
# add files of jsoncpp dependency
##################################
set(SOURCES_JSONCPP "dependencies/jsoncpp/src/dict_node.cpp" "dependencies/jsoncpp/src/id_node.cpp" "dependencies/jsoncpp/src/json.cpp" "dependencies/jsoncpp/src/json_exception.cpp" "dependencies/jsoncpp/src/list_node.cpp" "dependencies/jsoncpp/src/node.cpp" "dependencies/jsoncpp/src/text_node.cpp")

set(HEADERS_JSONCPP "dependencies/jsoncpp/src/dict_node.hpp" "dependencies/jsoncpp/src/id_node.hpp" "dependencies/jsoncpp/src/json.hpp" "dependencies/jsoncpp/src/json_exception.hpp" "dependencies/jsoncpp/src/list_node.hpp" "dependencies/jsoncpp/src/node.hpp" "dependencies/jsoncpp/src/text_node.hpp" "dependencies/jsoncpp/src/token.hpp")

add_library(jsoncpp SHARED ${SOURCES_JSONCPP})
add_library(jsoncpp_static STATIC ${SOURCES_JSONCPP})

install(TARGETS jsoncpp DESTINATION lib)
install(TARGETS jsoncpp_static DESTINATION lib)
install(FILES ${HEADERS_JSONCPP} DESTINATION include)
##############
# end jsoncpp
##############

# cppjit is header only, it is therefore not built

#################
# build examples
#################

set(SOURCES_UNROLL_LOOP "examples/unroll_example.cpp")
add_executable(unroll_example ${SOURCES_UNROLL_LOOP})
install(TARGETS unroll_example DESTINATION examples)

set(SOURCES_LOOP_NEST "examples/loop_nest_example.cpp")
add_executable(loop_nest_example ${SOURCES_LOOP_NEST})
install(TARGETS loop_nest_example DESTINATION examples)

set(SOURCES_UNROLL_LOOP_AUTOTUNE_INLINE "examples/unroll_loop_autotune_inline.cpp")
add_executable(unroll_example_autotune_inline ${SOURCES_UNROLL_LOOP_AUTOTUNE_INLINE})
target_link_libraries(unroll_example_autotune_inline dl)
install(TARGETS unroll_example_autotune_inline DESTINATION examples)

set(SOURCES_UNROLL_LOOP_AUTOTUNE "examples/unroll_loop_autotune.cpp")
add_executable(unroll_example_autotune ${SOURCES_UNROLL_LOOP_AUTOTUNE})
target_link_libraries(unroll_example_autotune dl)
install(TARGETS unroll_example_autotune DESTINATION examples)

#################
# end examples
#################

#################
# paper examples
#################

set(SOURCES_FMM_M2M_INTERACTIONS
  "paper/fmm_m2m_interactions/fmm_m2m_interactions.cpp"
  "paper/fmm_m2m_interactions/kernels/m2m_interactions.cpp"
  "paper/fmm_m2m_interactions/kernels/m2m_kernel.cpp"
  "paper/fmm_m2m_interactions/kernels/m2m_kernel.cpp"
  "paper/fmm_m2m_interactions/kernels/m2m_kernel_blocked_interaction.cpp"
  "paper/fmm_m2m_interactions/kernels/calculate_stencil.cpp"
  "paper/fmm_m2m_interactions/compute_factor.cpp"
  "paper/fmm_m2m_interactions/taylor.cpp"
)
add_executable(fmm_m2m_interactions ${SOURCES_FMM_M2M_INTERACTIONS})
install(TARGETS fmm_m2m_interactions DESTINATION paper)

#################
# end paper examples
#################
