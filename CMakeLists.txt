cmake_minimum_required(VERSION 3.25)
project(Awki)

# Put predefined targets into a specific folder
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_CMakePredefinedTargets")

# Set all targes to this folder by default
set(CMAKE_FOLDER "_External Dependencies")

# Set the configuration types that we are going to be using
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Adds the cmake folder that contains extra cmake scripts
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

# Find required libraries
find_package(Vulkan REQUIRED)

# Include ThirdParty projects
include(BuildExternalProjects)

# Configure compiler flags
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

	# Will show all warnings and treat them as errors but only on our projects.
	set(ALL_WARNINGS_AS_ERRORS "/external:anglebrackets /external:W0 /analyze:external- /WX /W4")

	# Per Target Compiler Settings
	set(CMAKE_CXX_FLAGS_DEBUG "/arch:AVX /MP /GR- /Od /wd4100 /wd4189 ${ALL_WARNINGS_AS_ERRORS}")
	set(CMAKE_CXX_FLAGS_RELEASE "/arch:AVX /MP /GR- /Ot ${ALL_WARNINGS_AS_ERRORS}")
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:EditAndContinue>")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	
	# Will show all warnings and treat them as error but only on our projects.
	set(ALL_WARNINGS_AS_ERRORS "-Wall -Wextra -Werror")

	# Per Target Compiler Settings
	set(CMAKE_CXX_FLAGS_DEBUG "-mavx -O0 -g -fno-rtti -Wno-unused-variable -Wno-unused-parameter ${ALL_WARNINGS_AS_ERRORS}")
	set(CMAKE_CXX_FLAGS_RELEASE "-mavx -O3 -fno-rtti ${ALL_WARNINGS_AS_ERRORS}")
endif()

# Base preprocessor definitions
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")

# Set ThirdParty include directories
include_directories(SYSTEM
	${Vulkan_INCLUDE_DIR}

	ThirdParty/fmt/include
	ThirdParty/glm
	ThirdParty/SDL/include
)

# Set ThirdParty link targets
list(APPEND LINK_TARGETS
	fmt::fmt
	SDL3-static
	Vulkan::Vulkan
)

# Add Engine library
add_subdirectory(Source/Engine)

# Gather Game project source files
file(GLOB_RECURSE GAME_SOURCE CONFIGURE_DEPENDS "Source/Game/*.cpp" "Source/Game/*.h")

# Create Game executable
add_executable(Game ${GAME_SOURCE})

# Set Game dependencies
add_dependencies(Game Engine)

# Set Game preprocessor definitions
target_compile_definitions(Game PRIVATE ${ENGINE_DEFINES})

# Link executable with libraries
target_link_libraries(Game PUBLIC Engine ${LINK_TARGETS})

# Remove the Game target from the default folder
set_target_properties(Game PROPERTIES FOLDER "")

# Enable C++20
target_compile_features(Game PUBLIC cxx_std_20)

# Include directories specific to the Game target
target_include_directories(Game PRIVATE Source/Game/.)

# Enable Visual Studio solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Visual Studio source grouping
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source/Game FILES ${GAME_SOURCE})

# MSVC Settings
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

	# Do not spawn a console when running on release
	set_target_properties(Game PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")

	# Set Visual Studio default startup project and debugging startup directory
	set_target_properties(Game PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ..)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Game)
endif()