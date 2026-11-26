cmake_minimum_required(VERSION 3.20)
project(hybrid-kem-c VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(liboqs REQUIRED
             HINTS $ENV{HOME}/Documents/job-hunt/tmp/oqs)

enable_testing()
add_executable(test_mlkem_roundtrip tests/test_mlkem_roundtrip.c)
target_link_libraries(test_mlkem_roundtrip PRIVATE OQS::oqs)
add_test(NAME mlkem_roundtrip COMMAND test_mlkem_roundtrip)
