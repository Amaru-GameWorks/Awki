cmake_minimum_required(VERSION 3.25)
project(Awki)

# Gather Engine project source files
file(GLOB_RECURSE ENGINE_SOURCE CONFIGURE_DEPENDS "*.cpp" "*.h")

# Add the Engine library
add_library(Engine ${ENGINE_SOURCE})

# Source grouping
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCE})

# Remove the Engine target from the default folder
set_target_properties(Engine PROPERTIES FOLDER "")

# Enable C++23
target_compile_features(Engine PUBLIC cxx_std_23)

#Define engine preprocessor definitions list
if(WIN32)
	list(APPEND ENGINE_DEFINES
		WIN32_LEAN_AND_MEAN
		VC_EXTRALEAN
		NOMINMAX
		NOGDI
	)
endif()

# Set preprocessor definitions
target_compile_definitions(Engine PUBLIC)

# Publish these include paths to projects depending on 'Engine'
target_include_directories(
	Engine PUBLIC
	.
)