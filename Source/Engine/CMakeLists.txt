cmake_minimum_required(VERSION 4.2)
project(Awki)

# Gather Engine project source files
file(GLOB_RECURSE ENGINE_SOURCE CONFIGURE_DEPENDS "*.cpp" "*.h")

# Add the Engine library
add_library(Engine ${ENGINE_SOURCE})

# Source grouping
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCE})

# Remove the Engine target from the default folder
set_target_properties(Engine PROPERTIES FOLDER "")

# Enable C++20
target_compile_features(Engine PUBLIC cxx_std_20)

# Set engine preprocessor definitions
list(APPEND ENGINE_DEFINES
	FMT_UNICODE=0
	VULKAN_HPP_NO_CONSTRUCTORS
	VULKAN_HPP_NO_NODISCARD_WARNINGS
	VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
	GLM_FORCE_INTRINSICS
	GLM_FORCE_DEPTH_ZERO_TO_ONE
	GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
)

if(WIN32)
	list(APPEND ENGINE_DEFINES
		WIN32_LEAN_AND_MEAN
		VC_EXTRALEAN
		NOMINMAX
		NOGDI
	)
endif()

# Set preprocessor definitions
target_compile_definitions(Engine PUBLIC ${ENGINE_DEFINES})

# Publish these include paths to projects depending on 'Engine'
target_include_directories(
	Engine PUBLIC
	.
)

set(ENGINE_DEFINES ${ENGINE_DEFINES} PARENT_SCOPE)