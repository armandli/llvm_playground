# Codegen library
set(CODEGEN_SOURCES
    # Add source files here when they exist
)

if(CODEGEN_SOURCES)
    add_library(codegen STATIC ${CODEGEN_SOURCES})
    target_link_libraries(codegen ${llvm_libs})
    if(TARGET util)
        target_link_libraries(codegen util)
    endif()
    if(TARGET lexer)
        target_link_libraries(codegen lexer)
    endif()
    if(TARGET parser)
        target_link_libraries(codegen parser)
    endif()
endif()