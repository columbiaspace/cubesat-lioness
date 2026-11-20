register_fprime_library(
    AUTOCODER_INPUTS "${CMAKE_CURRENT_LIST_DIR}/ImageProcessing.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/ImageProcessing.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/FrameProcessor.cpp"
)

if(BUILD_TESTING)
    find_package(PNG 1.6 REQUIRED)
    register_fprime_ut(
        AUTOCODER_INPUTS "${CMAKE_CURRENT_LIST_DIR}/ImageProcessing.fpp"
        SOURCES
            "${CMAKE_CURRENT_LIST_DIR}/test/ut/ImageProcessingTester.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/test/ut/Tests.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/test/ut/CameraFile.cpp"
        UT_AUTO_HELPERS
        DEPENDS PNG::PNG
    )
endif()
