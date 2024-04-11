cmake_minimum_required(VERSION 3.14)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# set the project name
project(VkBase VERSION 1.0
                       DESCRIPTION "Vulkan Base Framework"
                       LANGUAGES CXX)

add_subdirectory(external)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)



#FILE(GLOB_RECURSE SRCS "src/*.cpp" "src/*.h")
FILE(GLOB_RECURSE SRCS_FRAMEWORK
"src/framework/*.cpp"
"src/framework/*.h"
"resources/shaders/src/framework/*.vert"
"resources/shaders/src/framework/*.frag"
"resources/shaders/src/framework/*.glsl"
"resources/shaders/src/framework/*.comp"
"resources/shaders/src/framework/*.geom"
"resources/shaders/src/framework/*.rgen"
"resources/shaders/src/framework/*.rchit"
"resources/shaders/src/framework/*.rahit"
"resources/shaders/src/framework/*.rmiss"
)

FILE(GLOB_RECURSE SRCS_APPLICATIONS
"src/applications/*.cpp"
"src/applications/*.h"
"resources/shaders/src/applications/*.vert"
"resources/shaders/src/applications/*.frag"
"resources/shaders/src/applications/*.glsl"
"resources/shaders/src/applications/*.comp"
"resources/shaders/src/applications/*.geom"
"resources/shaders/src/applications/*.rgen"
"resources/shaders/src/applications/*.rchit"
"resources/shaders/src/applications/*.rahit"
"resources/shaders/src/applications/*.rmiss"
)



#message(STATUS "Print Sources:")
#message(STATUS ${SRCS})


MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
  IF(IS_DIRECTORY ${child})
  string(REPLACE "${curdir}/" "" child "${child}")
  LIST(APPEND dirlist "${child}")
  ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# Set compile definitions for shaders
MATH(EXPR SHADER_SRC_CNT_VAR "0")
SET(SHADER_SRC_DIRS_VAR "{")


SUBDIRLIST(SUBDIRS "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/src/framework")


FOREACH(subdir ${SUBDIRS})
  MATH(EXPR SHADER_SRC_CNT_VAR "${SHADER_SRC_CNT_VAR}+1")
  if(NOT ${SHADER_SRC_DIRS_VAR} STREQUAL "{")
    SET(SHADER_SRC_DIRS_VAR "${SHADER_SRC_DIRS_VAR},")
  endif()
  SET(SHADER_SRC_DIRS_VAR "${SHADER_SRC_DIRS_VAR} ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/src/framework/${subdir}")
ENDFOREACH()

SUBDIRLIST(SUBDIRS "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/src/applications")
FOREACH(subdir ${SUBDIRS})
  MATH(EXPR SHADER_SRC_CNT_VAR "${SHADER_SRC_CNT_VAR}+1")
  if(NOT ${SHADER_SRC_CNT_VAR} MATCHES "0")
    SET(SHADER_SRC_DIRS_VAR "${SHADER_SRC_DIRS_VAR},")
  endif()
  SET(SHADER_SRC_DIRS_VAR "${SHADER_SRC_DIRS_VAR} ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/src/applications/${subdir}")
ENDFOREACH()



SET(SHADER_SRC_DIRS_VAR "${SHADER_SRC_DIRS_VAR} }")
add_compile_definitions(SHADER_SRC_DIR="${SHADER_SRC_DIRS_VAR}")
message(STATUS "SHADER_SRC_DIRS:")
message(STATUS ${SHADER_SRC_DIRS_VAR})
add_compile_definitions(SHADER_SRC_CNT="${SHADER_SRC_CNT_VAR}")
message(STATUS "SHADER_SRC_CNT_VAR:")
message(STATUS ${SHADER_SRC_CNT_VAR})
add_compile_definitions(SHADER_SPV_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/spv")
add_compile_definitions(SHADER_LOG_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/log")


foreach(FILE ${SRCS_FRAMEWORK}) 
    # Get the directory of the source file
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    # Remove common directory prefix to make the group
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
    # Make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")
    # Group into "Source Files" and "Header Files"
    #if ("${FILE}" MATCHES ".*\\.cpp")
    #  set(GROUP "Source Files${GROUP}")
    #elseif("${FILE}" MATCHES ".*\\.h")
    #  set(GROUP "Header Files${GROUP}")
    #endif()
    source_group("${GROUP}" FILES "${FILE}")
endforeach()

foreach(FILE ${SRCS_APPLICATIONS}) 
    # Get the directory of the source file
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    # Remove common directory prefix to make the group
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
    # Make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")
    # Group into "Source Files" and "Header Files"
    #if ("${FILE}" MATCHES ".*\\.cpp")
    #  set(GROUP "Source Files${GROUP}")
    #elseif("${FILE}" MATCHES ".*\\.h")
    #  set(GROUP "Header Files${GROUP}")
    #endif()
    source_group("${GROUP}" FILES "${FILE}")
endforeach()


# we depend on vulkan 
find_package(Vulkan REQUIRED)



add_library(${PROJECT_NAME} STATIC
${SRCS_FRAMEWORK}
)

SET(target_list "${PROJECT_NAME}")
SUBDIRLIST(SUBDIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/applications")
FOREACH(subdir ${SUBDIRS})
  FILE(GLOB_RECURSE current_sources "src/applications/${subdir}/*.cpp" "src/applications/${subdir}/*.h")
  add_executable(${subdir} ${current_sources})
  LIST(APPEND target_list "${subdir}")
ENDFOREACH()



############################################################

if(UNIX)
install(TARGETS ${target_list} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
install(TARGETS ${target_list} DESTINATION bin)
endif()

set_target_properties (${target_list} PROPERTIES
    FOLDER C++/static
)



# set all glfw dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/GLFW/glfw-3.3.4)


set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${VkBase_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VkBase_VERSION_MINOR}")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set_target_properties( ${target_list} PROPERTIES RUNTIME_OUTPUT_DIRECTORY 
${PROJECT_BINARY_DIR})

set_target_properties( ${target_list} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
${PROJECT_BINARY_DIR})

FOREACH(target ${target_list})
  target_include_directories(${target} PUBLIC   ./src/ )
  target_include_directories(${target} PRIVATE ${Vulkan_INCLUDE_DIRS})
  target_link_libraries(${target} glfw)
  target_link_libraries(${target} Vulkan::Vulkan imgui stb glm tinyobjloader vma ktx csvwriter nlohmann_json)
  target_link_libraries(${target} ${CMAKE_DL_LIBS} Threads::Threads)
  if(NOT ${target} MATCHES  ${PROJECT_NAME})
    target_link_libraries(${target} ${PROJECT_NAME})
  endif()
ENDFOREACH()

if(UNIX)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
else()
endif()
