# fmt
set(FMT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/fmt")
add_subdirectory(${FMT_DIR} SYSTEM)

# SDL3
set(SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/SDL")
set(SDL_TEST OFF)
set(SDL_STATIC ON)
add_subdirectory(${SDL_DIR} SYSTEM)