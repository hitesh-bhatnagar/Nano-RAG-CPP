cmake_minimum_required(VERSION 3.14)
project(nano_rag)

set(CMAKE_CXX_STANDARD 17)

# this tells CMake download  library directly from github

include(FetchContent)
FetchContent_Declare(
    llama_cpp
    GIT_REPOSITORY https://github.com/ggerganov/llama.cpp
    GIT_TAG master
)
# This builds the library
FetchContent_MakeAvailable(llama_cpp)

# creates program similarly like g++ main.cpp -o nano_rag but portable and scalable
add_executable(nano_rag main.cpp)

##	links executable with llama.cpp library
target_link_libraries(nano_rag PRIVATE llama)	# PRIVATE => nano_rag uses llama but does not expose llama to others
