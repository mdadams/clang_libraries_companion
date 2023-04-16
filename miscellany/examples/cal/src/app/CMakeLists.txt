add_executable(cal_test)
target_sources(cal_test PRIVATE test.cpp)
target_link_libraries(cal_test cal ${llvm_libs} clangTooling Threads::Threads)
include_directories(BEFORE
  "${CMAKE_CURRENT_BINARY_DIR}/../lib/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/../lib/include"
  ${LLVM_INCLUDE_DIRS}
)
