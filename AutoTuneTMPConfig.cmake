include(CMakeFindDependencyMacro)
# find_package(Vc ${Vc_FIND_VERSION} NO_MODULE PATHS ${Vc_ROOT})
find_package(cppjit PATHS ${CPPJIT_ROOT})
include("${CMAKE_CURRENT_LIST_DIR}/AutoTuneTMPTargets.cmake")
# target_include_directories(AutoTuneTMP INTERFACE
#   $<BUILD_INTERFACE:${Vc_INCLUDE_DIR}>
#   )
