include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.12
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(doctest)
