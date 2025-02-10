cmake_minimum_required(VERSION 3.15...3.31)
project(pcre2)

if(NOT napi_build_version)
  set(napi_build_version 8)
endif()
add_compile_definitions(NAPI_VERSION=${napi_build_version})

set(PCRE2_SUPPORT_JIT ON)
set(PCRE2_NEWLINE ANYCRLF)
set(PCRE2_STATIC_PIC ON)
add_subdirectory(pcre2)

add_library(${PROJECT_NAME} SHARED
    src/pcre2.cpp
    ${CMAKE_JS_SRC}
)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
target_compile_definitions(${PROJECT_NAME} PRIVATE NODE_ADDON_API_CPP_EXCEPTIONS PCRE2_STATIC PCRE2_CODE_UNIT_WIDTH=8)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB} pcre2-8-static)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
  # Generate node.lib
  execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()
