cmake_minimum_required(VERSION 3.2)

project(cppjit)

include_directories(include)

set(CMAKE_CXX_FLAGS "-std=c++14 -g -Wall -Wextra -Wno-unused-parameter ${CMAKE_CXX_FLAGS}")

include_directories(src)
include_directories(dependencies/cppjit/include)

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

set(SOURCES_UNROLL_LOOP_AUTOTUNE "examples/unroll_loop_autotune.cpp")
add_executable(unroll_example_autotune ${SOURCES_UNROLL_LOOP_AUTOTUNE})
target_link_libraries(unroll_example_autotune dl)
install(TARGETS unroll_example_autotune DESTINATION examples)

#################
# end examples
#################
