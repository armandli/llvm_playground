# Lexer library
set(LEXER_SOURCES
    # Add source files here when they exist
)

if(LEXER_SOURCES)
    add_library(lexer STATIC ${LEXER_SOURCES})
    target_link_libraries(lexer ${llvm_libs})
    if(TARGET util)
        target_link_libraries(lexer util)
    endif()
endif()