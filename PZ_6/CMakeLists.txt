cmake_minimum_required(VERSION 3.16)
project(BigIntRand LANGUAGES CXX)

# --- компилятор и флаги ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Ворнинги (под Linux/GCC/Clang)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

# --- исполняемый файл ---
add_executable(bigint_demo
  main/main.cpp
  src/bigint.cpp
  # если появятся .cpp для энтропии/мерсенна — добавьте их сюда
)

# инклуды: /inc с заголовками bigint.h, entropy.h, mt19937.h
target_include_directories(bigint_demo
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# --- зависимости под Linux/WSL2 ---
if (UNIX)
  # X11 для чтения положения курсора
  find_package(X11)
  if (X11_FOUND)
    target_link_libraries(bigint_demo PRIVATE X11::X11)
    target_compile_definitions(bigint_demo PRIVATE HAVE_X11=1)
  else()
    message(WARNING "X11 not found: сбор энтропии с курсора будет недоступен (fallback).")
  endif()

  # Нити (sleep_for и т.п.)
  find_package(Threads REQUIRED)
  target_link_libraries(bigint_demo PRIVATE Threads::Threads)
endif()
