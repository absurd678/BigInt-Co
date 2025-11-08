cmake_minimum_required(VERSION 3.12)
project(pz3)

set(CMAKE_CXX_STANDARD 17)

# Поиск GMP
find_library(GMP_LIB gmp)
find_library(GMPXX_LIB gmpxx)

if(NOT GMP_LIB OR NOT GMPXX_LIB)
    message(FATAL_ERROR "GMP libraries not found. Install with: sudo apt install libgmp-dev libgmpxx-dev")
endif()

# Include директории
include_directories(inc)
include_directories(/usr/include)  # для GMP

# Библиотек
add_library(bigint src/bigint.cpp)
add_library(primtest src/primality_tests.cpp)

# Исполняемый файл
add_executable(pz3 main/main_practical3.cpp)

# Линковка
target_link_libraries(pz3 bigint primtest ${GMPXX_LIB} ${GMP_LIB})