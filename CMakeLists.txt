cmake_minimum_required(VERSION 3.0)
project(Drogi C)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

# Nie chcemy oglądać komunikatów o procentowym postępie kompilacji.
set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)

# Chcemy zobaczyć polecenia wywołane przez make.
set(CMAKE_VERBOSE_MAKEFILE ON)

# Ustawiamy wspólne opcje kompilowania dla wszystkich wariantów projektu.
#set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Werror -fsanitize=shift -fsanitize=integer-divide-by-zero  -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=null  -fsanitize=return -fsanitize=signed-integer-overflow -ggdb3")
set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Werror")
# Domyślne opcje dla wariantów Release i Debug są sensowne.
# Jeśli to konieczne, ustawiamy tu inne.
#set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -pg")
set(CMAKE_C_FLAGS_DEBUG "-ggdb3")

# Wskazujemy pliki źródłowe.

set(SOURCE_FILES
    src/dictionary.c
    src/dictionary.h
    src/list.c
    src/list.h
    src/map.c
    src/map.h
    src/map_main.c
    src/utils.c
    src/utils.h
    src/vector.c
    src/vector.h
    src/status.h
    src/shortest_paths.c
    src/shortest_paths.h
    src/map_struct.h
    src/parser.c
    src/parser.h
    src/map_text_interface.c
    src/map_text_interface.h
    src/queue.c
    src/queue.h
    #src/malloc_test.c
    #src/malloc_test.h
    )

# Wskazujemy plik wykonywalny.
add_executable(map ${SOURCE_FILES})

# Dodajemy obsługę Doxygena: sprawdzamy, czy jest zainstalowany i jeśli tak to:
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # Wskazujemy lokalizacją pliku konfiguracyjnego i podajemy jego docelową lokalizację w folderze, gdzie następuje kompilacja.
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    # Dodajemy cel doc: użycie make doc spowoduje wywołanie doxygena z odpowiednim plikiem konfiguracyjnym w folderze kompilacji.
    # Na wyjście zostanie wypisany odpowiedni komentarz.
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
endif (DOXYGEN_FOUND)
