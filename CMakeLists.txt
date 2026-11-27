cmake_minimum_required(VERSION 3.21)
project(hybrid-kem-c VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(liboqs 0.16.0 REQUIRED)

enable_testing()
add_executable(test_mlkem_roundtrip tests/test_mlkem_roundtrip.c)
target_link_libraries(test_mlkem_roundtrip PRIVATE OQS::oqs)
add_test(NAME mlkem_roundtrip COMMAND test_mlkem_roundtrip)

# Lint targets (QUAL-06, QUAL-07)
find_program(CPPCHECK_EXE cppcheck)
find_program(CLANG_TIDY_EXE clang-tidy)
set(MISRA_RULE_TEXTS "" CACHE FILEPATH "Local MISRA headlines file")

get_target_property(OQS_INC OQS::oqs INTERFACE_INCLUDE_DIRECTORIES)
list(TRANSFORM OQS_INC PREPEND "-I")

# MISRA scope: library sources only, from M1 (CODING-STANDARD section 2)
set(MISRA_SOURCES "")
# clang-tidy cert-*: tests and M0 program
set(TIDY_SOURCES ${CMAKE_SOURCE_DIR}/tests/test_mlkem_roundtrip.c)

if(CPPCHECK_EXE AND MISRA_SOURCES)
  file(WRITE ${CMAKE_BINARY_DIR}/misra.json
    "{\"script\": \"misra.py\", \"args\": [\"--rule-texts=${MISRA_RULE_TEXTS}\"]}\n")
  add_custom_target(misra
    COMMAND ${CPPCHECK_EXE} --std=c11 --enable=style
            --addon=${CMAKE_BINARY_DIR}/misra.json
            -I ${CMAKE_SOURCE_DIR}/include ${OQS_INC} ${MISRA_SOURCES}
    COMMAND_EXPAND_LISTS VERBATIM)
endif()

if(CLANG_TIDY_EXE)
  add_custom_target(tidy
    COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR}
            --checks=-*,cert-* ${TIDY_SOURCES}
    COMMAND_EXPAND_LISTS VERBATIM)
endif()
