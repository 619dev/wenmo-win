cmake_minimum_required(VERSION 3.20)
project(WenmoWin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(wenmo_core
    src/core/InputEngine.cpp
)
target_include_directories(wenmo_core PUBLIC include)
target_compile_features(wenmo_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(wenmo_core PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(wenmo_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    add_library(wenmo_tsf SHARED
        src/windows/ClassFactory.cpp
        src/windows/DllMain.cpp
        src/windows/Globals.cpp
        src/windows/TextService.cpp
        src/windows/WenmoTsf.def
    )
    target_include_directories(wenmo_tsf PRIVATE include src/windows)
    target_compile_features(wenmo_tsf PRIVATE cxx_std_20)
    target_compile_definitions(wenmo_tsf PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_compile_options(wenmo_tsf PRIVATE /W4 /permissive- /utf-8)
    target_link_libraries(wenmo_tsf PRIVATE wenmo_core msctf ole32 uuid)
    set_target_properties(wenmo_tsf PROPERTIES
        OUTPUT_NAME "WenmoTsf"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    add_custom_command(TARGET wenmo_tsf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:wenmo_tsf>/resources"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/resources/cedict_pinyin.tsv"
            "$<TARGET_FILE_DIR:wenmo_tsf>/resources/cedict_pinyin.tsv"
    )
    install(TARGETS wenmo_tsf RUNTIME DESTINATION . LIBRARY DESTINATION .)
    install(FILES resources/cedict_pinyin.tsv DESTINATION resources)
endif()

enable_testing()
add_executable(wenmo_core_tests tests/InputEngineTests.cpp)
target_link_libraries(wenmo_core_tests PRIVATE wenmo_core)
target_compile_definitions(wenmo_core_tests PRIVATE
    WENMO_TEST_DICTIONARY="${CMAKE_CURRENT_SOURCE_DIR}/resources/cedict_pinyin.tsv"
)
add_test(NAME wenmo_core_tests COMMAND wenmo_core_tests)
