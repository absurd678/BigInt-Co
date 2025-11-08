cmake_minimum_required(VERSION 3.12)
project(pz4)

set(CMAKE_CXX_STANDARD 11)

# Поиск GMP
find_library(GMP_LIB gmp)
find_library(GMPXX_LIB gmpxx)

if(NOT GMP_LIB OR NOT GMPXX_LIB)
    message(FATAL_ERROR "GMP libraries not found. Install with: sudo apt install libgmp-dev libgmpxx-dev")
endif()

# Include директории
include_directories(inc)
include_directories(/usr/include)

# Одна библиотека со всеми исходниками
add_library(pz4_lib 
    src/bigint.cpp
    src/deterministic_primality.cpp 
    src/polynomial.cpp
)
target_link_libraries(pz4_lib ${GMPXX_LIB} ${GMP_LIB})

# Исполняемый файл
add_executable(pz4 main/main_practical4.cpp)
target_link_libraries(pz4 pz4_lib)