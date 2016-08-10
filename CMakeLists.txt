cmake_minimum_required (VERSION 2.8)
project (ssp21cpp)
set(SSP21CPP_VERSION 0.0.1)

include(${PROJECT_SOURCE_DIR}/cmake/settings.cmake)

# libsodium include dir
if(DEFINED SSP21_LIBSODIUM_INCLUDE_DIR)
  include_directories(${SSP21_LIBSODIUM_INCLUDE_DIR}) 
endif()

# libsodium lib dir
if(DEFINED SSP21_LIBSODIUM_LIB_DIR)
  link_directories(${SSP21_LIBSODIUM_LIB_DIR}) 
endif()

include_directories(./cpp/libs/include) 

# ---- openpal library ----
file(GLOB_RECURSE sodiumbackend_SRC ./cpp/libs/src/sodiumbackend/*.cpp ./cpp/libs/src/sodiumbackend/*.h ./cpp/libs/include/sodiumbackend/*.h)
add_library(sodiumbackend ${LIB_TYPE} ${sodiumbackend_SRC})
install(TARGETS sodiumbackend DESTINATION lib)
set_target_properties(sodiumbackend PROPERTIES FOLDER libs)

