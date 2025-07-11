# Parser library
set(PARSER_SOURCES
    # Add source files here when they exist
)

if(PARSER_SOURCES)
    add_library(parser STATIC ${PARSER_SOURCES})
    target_link_libraries(parser ${llvm_libs})
    if(TARGET util)
        target_link_libraries(parser util)
    endif()
    if(TARGET lexer)
        target_link_libraries(parser lexer)
    endif()
endif()