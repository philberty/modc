project(Cmod)
cmake_minimum_required(VERSION 2.6)
add_subdirectory(src)

set(CMAKE_MODULE_PATH "cmake/")
find_package(BISON)
find_package(FLEX)

BISON_TARGET(ModParser src/ss-parser.y src/ss-parser.c)
FLEX_TARGET(ModScanner src/ss-lexer.l src/ss-lexer.c)
ADD_FLEX_BISON_DEPENDENCY(ModScanner ModParser)

include_directories (include)
ADD_LIBRARY(parser STATIC 
  ${BISON_ModParser_OUTPUTS}
  ${FLEX_ModScanner_OUTPUTS}
  )

set (CMOD_VERSION 0.01)
set (CMOD_MACHINE_TYPE ${CMAKE_SYSTEM_PROCESSOR})
set (CMOD_BUG_REPORT "redbrain@gcc.gnu.org")
set (CMOD_SYSTEM ${CMAKE_HOST_SYSTEM})
# does this system provide the log and exp functions?
include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
include (${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)

CHECK_INCLUDE_FILES (stdio.h HAVE_STDIO)
CHECK_INCLUDE_FILES (stdlib.h HAVE_STDLIB)
CHECK_INCLUDE_FILES (time.h HAVE_TIME)
CHECK_INCLUDE_FILES (stdbool.h HAVE_BOOL)
CHECK_INCLUDE_FILES (string.h HAVE_STRING)

check_function_exists (strlen CMOD_HAVE_STRLEN)
check_function_exists (strdup CMOD_HAVE_STRDUP)
check_function_exists (fopen HAVE_FOPEN)
check_function_exists (fclose HAVE_FCLOSE)

check_function_exists (malloc HAVE_MALLOC)
check_function_exists (free HAVE_FREE)

option (CMOD_DEBUG
  "Compile Debug Symbols" OFF)

IF (CMOD_DEBUG)
  SET (C_DEBUG 1)
ELSE (CMOD_DEBUG)
  SET (C_DEBUG 0)
ENDIF (CMOD_DEBUG)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "src/config.h.in.cmake"
  "src/config.h"
  )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")
