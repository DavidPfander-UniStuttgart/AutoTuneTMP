include(CMakeFindDependencyMacro)
find_package(Vc ${Vc_FIND_VERSION} QUIET NO_MODULE PATHS ${Vc_ROOT})
find_package(cppjit PATHS ${CPPJIT_ROOT})
include("${CMAKE_CURRENT_LIST_DIR}/AutoTuneTMPTargets.cmake")
