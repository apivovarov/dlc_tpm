cmake_minimum_required (VERSION 3.17)
project(dlc_tpm VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG db351ffbbadc6c4e9239daaa26e9aefa9f0ec82d)
FetchContent_MakeAvailable(cpr)

#include_directories("${PROJECT_SOURCE_DIR}/include")
#link_directories(${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR})


FILE(GLOB DLC_TPM_SRC
    "src/*.cc"
)

add_executable(dlc_tpm ${DLC_TPM_SRC})
#target_include_directories(dlc_tpm INTERFACE ${cpr_SOURCE_DIR}/include)
target_link_libraries(dlc_tpm PRIVATE cpr::cpr)
