add_executable(
        gemini_response_tests
        GeminiResponseTests.cpp
        ../src/ai/GeminiResponse.cpp
        ../src/ai/GeminiResponse.h
)

target_include_directories(
        gemini_response_tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(
        gemini_response_tests
        PRIVATE
        Drogon::Drogon
)

target_compile_features(
        gemini_response_tests
        PRIVATE
        cxx_std_20
)

if(MSVC)
    target_compile_options(
            gemini_response_tests
            PRIVATE
            /W4
            /permissive-
    )
else()
    target_compile_options(
            gemini_response_tests
            PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

add_test(
        NAME gemini_response_tests
        COMMAND gemini_response_tests
)