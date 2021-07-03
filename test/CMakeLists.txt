cmake_minimum_required(VERSION 3.10)

add_custom_target(build_test)
add_dependencies(build_test build_commet)
find_package(GTest REQUIRED)
include(GoogleTest)
add_executable(test_commet 
            ./comet_ii/test_svc.cc
            ./comet_ii/test.cpp
            ./assembler/test_assembler.cc
            ./reader/test_reader.cc
    )
target_link_libraries(test_commet commetII GTest::GTest GTest::Main pthread)
include_directories(${PROJECT_SOURCE_DIR}/src ${GTEST_INCLUDE_DIRS})

add_dependencies(build_test test_commet)

# Google Testの各テストケースごとにCTestのテストを作成する
# gtest_add_tests(TARGET TestCommet)