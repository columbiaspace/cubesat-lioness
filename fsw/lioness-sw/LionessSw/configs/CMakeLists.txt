####
# configs/CMakeLists.txt:
#
# Project-level configuration overrides. Files listed under CONFIGURATION_OVERRIDES
# replace the equivalently-named file from the fprime core configuration modules
# when the build assembles the configuration tree.
#
# See: lib/fprime/docs/user-manual/design-patterns/subtopologies.md
####
register_fprime_config(
    "LionessSwConfig"
    CONFIGURATION_OVERRIDES
        "${CMAKE_CURRENT_LIST_DIR}/ComCcsdsConfig.fpp"
    INTERFACE
)
