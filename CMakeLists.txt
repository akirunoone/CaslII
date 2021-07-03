cmake_minimum_required(VERSION 3.1)

project(Casl C CXX)

set(CMAKE_CXX_STANDARD 17)

if (MSYS OR MINGW)
   message("-- [BUILD] MSYS/MinGW use gcc")
   set(CMAKE_C_COMPILER "gcc")
   set(CMAKE_CXX_COMPILER "g++")
elseif(UNIX)
   message("-- [BUILD] UNIX use gcc")
   set(CMAKE_C_COMPILER "gcc")
   set(CMAKE_CXX_COMPILER "g++")
   #profileのためのオプション
   #set(CMAKE_C_COMPILER "clang")
   #set(CMAKE_CXX_COMPILER "clang++")   
   #profileのためのオプション
   #set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-instr-generate -fcoverage-mapping")
   #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate")
else()
   add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
endif()

#add_definitions(-D_GLIBCXX_USE_CXX17_ABI=0)
if(NOT WIN32)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0") 
endif()

include_directories(./src)

add_subdirectory(src)

if(NOT WIN32)
    add_subdirectory(test)
endif()

#target build_casl
add_custom_target(build_casl)
add_dependencies(build_casl build_commet)
add_executable(casl
        ./src/main.cc
    )
target_link_libraries(casl commetII)
add_dependencies(build_casl casl)

