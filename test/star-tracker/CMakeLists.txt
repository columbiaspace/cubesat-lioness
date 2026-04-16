cmake_minimum_required(VERSION 3.16)
project(star-tracker-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(star_tracker_test star_tracker_test.cpp star_id.cpp)

# Copy the executable to the source directory after every build
add_custom_command(TARGET star_tracker_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:star_tracker_test>
        ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Copying executable to project root..."
)
