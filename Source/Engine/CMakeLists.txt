cmake_minimum_required(VERSION 3.25)
project(Awki)

# Gather Engine project source files
file(GLOB_RECURSE ENGINE_SOURCE CONFIGURE_DEPENDS "*.cpp" "*.h")

# Add VisualStudio Natvis files
if (CMAKE_GENERATOR MATCHES "Visual Studio")
	set(NATVIS_FILES
		../../ThirdParty/glm/util/glm.natvis
	)
else()
	set(NATVIS_FILES)
endif()

# Add the Engine library
add_library(Engine ${ENGINE_SOURCE} ${NATVIS_FILES})

# Source grouping
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCE})

# Remove the Engine target from the default folder
set_target_properties(Engine PROPERTIES FOLDER "")

# Enable C++23
target_compile_features(Engine PUBLIC cxx_std_23)

# Set ThirdParty preprocessor definitions
list(APPEND ENGINE_DEFINES
	GLM_FORCE_INTRINSICS
	GLM_FORCE_DEPTH_ZERO_TO_ONE
	GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
	
	VULKAN_HPP_NO_CONSTRUCTORS
	VULKAN_HPP_NO_NODISCARD_WARNINGS
	VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

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
target_compile_definitions(Engine PUBLIC ${ENGINE_DEFINES})

# Publish these include paths to projects depending on 'Engine'
target_include_directories(
	Engine PUBLIC
	.
)

# Publish Engine preprocessor definitions to parent project
set(ENGINE_DEFINES ${ENGINE_DEFINES} PARENT_SCOPE)