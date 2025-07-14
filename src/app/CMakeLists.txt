# Application executables

# Kaleidoscope parser executable
add_executable(kaleidoscope kaleidoscope.cpp)
# kaleidoscope.cpp is self-contained and only uses standard C++ libraries
target_compile_features(kaleidoscope PRIVATE cxx_std_17)
# Set output directory to bin
set_target_properties(kaleidoscope PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Codegen executable
add_executable(codegen codegen.cpp)
# codegen.cpp uses LLVM libraries
target_compile_features(codegen PRIVATE cxx_std_17)
target_link_libraries(codegen ${llvm_libs})
# Set output directory to bin
set_target_properties(codegen PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Optional: Add other executables when source files exist
set(APP_SOURCES
    # Add source files here when they exist
)

if(APP_SOURCES)
    # Example executable (uncomment and modify when source files exist)
    # add_executable(llvm_playground_app ${APP_SOURCES})
    # target_link_libraries(llvm_playground_app ${llvm_libs} ${Boost_LIBRARIES})
    # if(TARGET util)
    #     target_link_libraries(llvm_playground_app util)
    # endif()
    # if(TARGET lexer)
    #     target_link_libraries(llvm_playground_app lexer)
    # endif()
    # if(TARGET parser)
    #     target_link_libraries(llvm_playground_app parser)
    # endif()
    # if(TARGET codegen)
    #     target_link_libraries(llvm_playground_app codegen)
    # endif()
endif()