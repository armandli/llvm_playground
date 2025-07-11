# Utility library
set(UTIL_SOURCES
    # Add source files here when they exist
)

if(UTIL_SOURCES)
    add_library(util STATIC ${UTIL_SOURCES})
    target_link_libraries(util ${llvm_libs})
endif()