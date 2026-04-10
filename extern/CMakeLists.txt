################################################################################
# Check for the static analyzers

find_program(CODECHECKER_BIN NAMES CodeChecker)
if(NOT CODECHECKER_BIN)
    message(
        FATAL_ERROR
        "CodeChecker was not found in PATH. Install CodeChecker before configuring this project."
    )
endif()

set(STATIC_ANALYZER_CANDIDATES
    cppcheck
    clang-tidy
    scan-build
)

set(STATIC_ANALYZER_FOUND FALSE)
set(STATIC_ANALYZER_FOUND_NAMES "")
foreach(STATIC_ANALYZER IN LISTS STATIC_ANALYZER_CANDIDATES)
    find_program(STATIC_ANALYZER_BIN NAMES ${STATIC_ANALYZER})
    if(STATIC_ANALYZER_BIN)
        set(STATIC_ANALYZER_FOUND TRUE)
        list(APPEND STATIC_ANALYZER_FOUND_NAMES "${STATIC_ANALYZER}")
    endif()
endforeach()

if(NOT STATIC_ANALYZER_FOUND)
    message(
        FATAL_ERROR
        "No supported static analyzer was found in PATH. Install at least one of: "
        "cppcheck, clang-tidy, clang-analyzer/scan-build."
    )
endif()

list(JOIN STATIC_ANALYZER_FOUND_NAMES ", " STATIC_ANALYZER_FOUND_NAMES_TEXT)
message(STATUS "Found CodeChecker: ${CODECHECKER_BIN}")
message(STATUS "Found static analyzer(s): ${STATIC_ANALYZER_FOUND_NAMES_TEXT}")

################################################################################
# Download the library for the unit tests

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.12
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(doctest)
