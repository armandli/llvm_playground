# Codegen library
set(CODEGEN_SOURCES
    codegen.cpp
)

add_library(codegenlib STATIC ${CODEGEN_SOURCES})
target_compile_features(codegenlib PRIVATE cxx_std_17)
target_link_libraries(codegenlib ${llvm_libs})
if(TARGET util)
    target_link_libraries(codegenlib util)
endif()
if(TARGET lexer)
    target_link_libraries(codegenlib lexer)
endif()
if(TARGET parser)
    target_link_libraries(codegenlib parser)
endif()