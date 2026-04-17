# This CMake file is intended to register project-wide objects.
# This allows for reuse between deployments, or other projects.

# Project-level configuration overrides. Must be registered BEFORE anything that
# consumes the overridden core config modules (e.g. the core ComCcsds subtopology
# and any project-level subtopologies that share its configuration).
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/configs")

# Project-wide reusable components and ports
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Components")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Ports")

# Project-level subtopologies
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/ComCcsdsRoarFM")

# Deployments
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/FlatSat")
