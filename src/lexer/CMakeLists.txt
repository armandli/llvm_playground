# Lexer library
set(LEXER_SOURCES
    lexer.cpp
)

add_library(lexer STATIC ${LEXER_SOURCES})
target_compile_features(lexer PRIVATE cxx_std_17)
if(TARGET util)
    target_link_libraries(lexer util)
endif()