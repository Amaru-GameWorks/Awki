cmake_minimum_required(VERSION 3.25.0)

file(GLOB_RECURSE SHADERS CONFIGURE_DEPENDS "Shaders/*.slang")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SHADERS})

add_custom_target(Resources SOURCES ${SHADERS})
set_target_properties(Resources PROPERTIES FOLDER "")