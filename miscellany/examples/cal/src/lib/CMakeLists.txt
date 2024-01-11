set(headers
  include/cal/main.hpp
  include/cal/utility.hpp
)
set(sources
  utility.cpp
)

add_library(cal ${headers} ${sources})
#target_compile_definitions(cal PRIVATE ${LLVM_DEFINITIONS})
target_include_directories(cal BEFORE PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  #"${CMAKE_CURRENT_BINARY_DIR}/include"
  #${LLVM_INCLUDE_DIRS}
)
target_link_libraries(cal PRIVATE Boost::filesystem)
target_link_libraries(cal PRIVATE ClangFoo::clangcpp ClangFoo::llvm)
target_link_libraries(cal PRIVATE ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS cal
  EXPORT CALTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(EXPORT CALTargets
  NAMESPACE CAL::
  FILE CALTargets.cmake
  DESTINATION "${CAL_CONFIG_DIR}"
)
install(FILES
  ${headers}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cal"
)
export(EXPORT CALTargets
  FILE "${CMAKE_BINARY_DIR}/build/cmake/CALTargets.cmake"
)
