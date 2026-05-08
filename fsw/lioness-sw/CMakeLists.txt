####
# This sets up the build system for the 'LionessSw' project, including
# components and deployments from project.cmake. In addition, it imports the core F Prime components.
####

cmake_minimum_required(VERSION 3.24.2)
project(LionessSw C CXX)

###
# F' Core Setup
# This includes all of the F prime core components, and imports the make-system.
###
include("${CMAKE_CURRENT_LIST_DIR}/lib/fprime/cmake/FPrime.cmake")
# NOTE: register custom targets between these two lines
fprime_setup_included_code()

# Everything under the LionessSw namespace: project-wide components and ports,
# project-level configuration overrides, project-level subtopologies, and the
# FlatSat deployment.
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/LionessSw")
