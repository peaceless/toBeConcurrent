cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_STANDARD 11)
# set the project name and version
project(WebServer)

# add pthread add_libray
link_libraries(-lpthread)
link_libraries(-ldl)
link_libraries(${PROJECT_SOURCE_DIR}/lib/libssl.a)
link_libraries(${PROJECT_SOURCE_DIR}/lib/libcrypto.a)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})

# set execute file to path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./lib)

# add dir as a library
aux_source_directory(${PROJECT_SOURCE_DIR}/event EVENT)
#add_library(eventLib SHARED ${EVENT})

aux_source_directory(./httpUnit/fast_cgi FCGI)
aux_source_directory(${PROJECT_SOURCE_DIR}/httpUnit HTTPUNIT)
#add_library(httpLib SHARED ${HTTPUNIT})
set(ALLUNIT ${EVENT} ${HTTPUNIT} ${FCGI})
add_library(lib SHARED ${ALLUNIT})
aux_source_directory(./main WEBSERVER)
# add the executable
add_executable(WebServer ${WEBSERVER})
target_link_libraries(WebServer lib)

#target_link_libraries(WebServer httpLib)
#target_link_libraries(WebServer eventLib)
