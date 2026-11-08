cmake_minimum_required(VERSION 3.14)
project(Wizards CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS src/*.cpp)
add_executable(Wizards ${SRCS})
target_include_directories(Wizards PRIVATE src)
if(MINGW)
  target_link_options(Wizards PRIVATE -static)
endif()

# Ship content/ and packs/ next to the executable.
add_custom_command(TARGET Wizards POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/content $<TARGET_FILE_DIR:Wizards>/content
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/packs $<TARGET_FILE_DIR:Wizards>/packs)
