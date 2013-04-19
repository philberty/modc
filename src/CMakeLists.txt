include_directories (../include)

SET (SRC_LIST
  mm-cmod.c
  dd-util.c
  dd-vec.c
  ii-toplev.c
  ii-mod-dot.c
)

IF (CMAKE_HOST_UNIX)
  IF (CMAKE_HOST_APPLE)
    SET (CMOD_CRAPLE 1)
  ENDIF (CMAKE_HOST_APPLE)
  SET (SRC_LIST ${SRC_LIST} bb-unix-i386.c)
ENDIF (CMAKE_HOST_UNIX)

IF (CMAKE_HOST_WIN32)
  SET (SRC_LIST ${SRC_LIST} bb-win32-i386.c)
ENDIF (CMAKE_HOST_WIN32)

add_executable (cmod
  ${SRC_LIST}
)
TARGET_LINK_LIBRARIES (cmod parser)