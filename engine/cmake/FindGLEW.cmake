include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(GLEW_INCLUDE_DIR GL/glew.h)
mark_as_advanced(GLEW_INCLUDE_DIR)
set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})

find_library(GLEW_SHARED_LIBRARY_RELEASE NAMES GLEW glew)
find_library(GLEW_STATIC_LIBRARY_RELEASE NAMES GLEWs glews)

select_library_configurations(GLEW_SHARED)
select_library_configurations(GLEW_STATIC)

if(NOT GLEW_USE_STATIC_LIBS)
  set(GLEW_LIBRARIES ${GLEW_SHARED_LIBRARY})
else()
  set(GLEW_LIBRARIES ${GLEW_STATIC_LIBRARY})
endif()

if(EXISTS "${GLEW_INCLUDE_DIR}/GL/glew.h")
  file(STRINGS "${GLEW_INCLUDE_DIR}/GL/glew.h" _contents REGEX "^VERSION_.+ [0-9]+")
  if(_contents)
    string(REGEX REPLACE ".*VERSION_MAJOR[ \t]+([0-9]+).*" "\\1" GLEW_VERSION_MAJOR "${_contents}")
    string(REGEX REPLACE ".*VERSION_MINOR[ \t]+([0-9]+).*" "\\1" GLEW_VERSION_MINOR "${_contents}")
    string(REGEX REPLACE ".*VERSION_MICRO[ \t]+([0-9]+).*" "\\1" GLEW_VERSION_MICRO "${_contents}")
    set(GLEW_VERSION "${GLEW_VERSION_MAJOR}.${GLEW_VERSION_MINOR}.${GLEW_VERSION_MICRO}")
  endif()
endif()

find_package_handle_standard_args(GLEW
  REQUIRED_VARS GLEW_LIBRARIES GLEW_INCLUDE_DIRS
  VERSION_VAR GLEW_VERSION)

if(NOT TARGET GLEW::glew AND NOT GLEW_USE_STATIC_LIBS)
  add_library(GLEW::glew UNKNOWN IMPORTED)
  set_target_properties(GLEW::glew PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIRS}")
  if(GLEW_SHARED_LIBRARY_RELEASE)
    set_target_properties(GLEW::glew PROPERTIES
      IMPORTED_LOCATION "${GLEW_SHARED_LIBRARY_RELEASE}")
  endif()
endif()

if(NOT TARGET GLEW::glew_s AND GLEW_USE_STATIC_LIBS)
  add_library(GLEW::glew_s STATIC IMPORTED)
  set_target_properties(GLEW::glew_s PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIRS}")
  if(GLEW_STATIC_LIBRARY_RELEASE)
    set_target_properties(GLEW::glew_s PROPERTIES
      IMPORTED_LOCATION "${GLEW_STATIC_LIBRARY_RELEASE}")
  endif()
endif()

if(NOT TARGET GLEW::GLEW)
  add_library(GLEW::GLEW UNKNOWN IMPORTED)
  set_target_properties(GLEW::GLEW PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIRS}")
  if(TARGET GLEW::glew)
    set_target_properties(GLEW::GLEW PROPERTIES
      IMPORTED_LOCATION "${GLEW_SHARED_LIBRARY_RELEASE}")
  elseif(TARGET GLEW::glew_s)
    set_target_properties(GLEW::GLEW PROPERTIES
      IMPORTED_LOCATION "${GLEW_STATIC_LIBRARY_RELEASE}")
  endif()
endif()
