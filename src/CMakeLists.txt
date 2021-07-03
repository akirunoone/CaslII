cmake_minimum_required(VERSION 3.10)

#target build_commet
add_custom_target(build_commet)
add_library(commetII STATIC 
            comet_ii.cc
            assem_mem.cc
            reader.cc
            assembler.cc
            debugger.cc
            builder.cc
    )
add_dependencies(build_commet commetII)

