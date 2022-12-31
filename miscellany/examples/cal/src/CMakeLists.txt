set(headers
  include/cal/main.hpp
  include/cal/utility.hpp
)
set(sources
  utility.cpp
)

add_library(cal)

target_sources(cal PRIVATE ${sources})
target_sources(cal PUBLIC "${headers}")

target_compile_definitions(cal PRIVATE ${LLVM_DEFINITIONS})
target_include_directories(cal BEFORE PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_BINARY_DIR}/include"
  ${LLVM_INCLUDE_DIRS}
)
target_link_libraries(cal PRIVATE ${llvm_libs} clangTooling
  libclang Boost::filesystem)

target_link_libraries(cal PRIVATE
  ${llvm_libs} clangTooling libclang)

install(TARGETS cal
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES
  ${headers}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cal"
)
