cmake_minimum_required(VERSION 3.15)

project(tubes LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Opsional: aktifkan AddressSanitizer & UndefinedBehaviorSanitizer (Linux/macOS/WSL)
# Jalankan dengan: cmake -S . -B build -DENABLE_SANITIZERS=ON
option(ENABLE_SANITIZERS "Enable ASan and UBSan for tests" OFF)

# ==================================================
# Core Library
# ==================================================
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

add_library(project_core STATIC ${CORE_SOURCES})
target_include_directories(project_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# ==================================================
# Main Executable
# ==================================================
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE project_core)

# ==================================================
# Fetch Google Test ke folder vendor/googletest
# ==================================================
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
  GIT_SHALLOW    TRUE
  SOURCE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/vendor/googletest"
)

# Mencegah konflik runtime library pada Windows (MSVC)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# ==================================================
# Unit Tests (Google Test + CTest)
# ==================================================
enable_testing()
include(GoogleTest)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")

foreach(TEST_FILE IN LISTS TEST_SOURCES)
  get_filename_component(TEST_NAME "${TEST_FILE}" NAME_WE)
  add_executable(${TEST_NAME}_test "${TEST_FILE}")
  target_link_libraries(${TEST_NAME}_test PRIVATE project_core GTest::gtest_main)
  set_target_properties(${TEST_NAME}_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
  )

  if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(project_core PRIVATE -g -fsanitize=address,undefined)
    target_compile_options(${TEST_NAME}_test PRIVATE -g -fsanitize=address,undefined)
    target_link_options(${TEST_NAME}_test PRIVATE -fsanitize=address,undefined)
  endif()

  if(MINGW)
    target_link_options(${TEST_NAME}_test PRIVATE -static -static-libgcc -static-libstdc++)
  endif()

  gtest_discover_tests(${TEST_NAME}_test
    DISCOVERY_MODE PRE_TEST
    DISCOVERY_TIMEOUT 60
  )
endforeach()