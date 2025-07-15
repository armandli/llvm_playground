# Parser library
set(PARSER_SOURCES
    parser.cpp
)

add_library(parser STATIC ${PARSER_SOURCES})
target_compile_features(parser PRIVATE cxx_std_17)
if(TARGET util)
    target_link_libraries(parser util)
endif()
if(TARGET lexer)
    target_link_libraries(parser lexer)
endif()