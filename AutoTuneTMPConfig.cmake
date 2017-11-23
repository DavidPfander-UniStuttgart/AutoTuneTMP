include(CMakeFindDependencyMacro)
message(WARNING "CPPJIT_ROOT: ${CPPJIT_ROOT}")
find_package(cppjit PATHS ${CPPJIT_ROOT})
include("${CMAKE_CURRENT_LIST_DIR}/AutoTuneTMPTargets.cmake")
