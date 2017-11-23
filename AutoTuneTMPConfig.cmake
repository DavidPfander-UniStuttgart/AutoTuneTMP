include(CMakeFindDependencyMacro)
find_package(cppjit PATHS ${CPPJIT_ROOT})
include("${CMAKE_CURRENT_LIST_DIR}/AutoTuneTMPTargets.cmake")
