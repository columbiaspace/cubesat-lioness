####
# F Prime CMakeLists.txt:
#
# SOURCES: combined list of source and autocoding files
# AUTOCODER_INPUTS: list of FPP files
####

register_fprime_library(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/UartReceiver.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/UartReceiver.cpp"
)
