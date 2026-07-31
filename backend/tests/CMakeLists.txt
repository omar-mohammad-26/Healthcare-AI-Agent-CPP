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


add_executable(
        tool_registry_tests
        ToolRegistryTests.cpp
        ../src/agent/ToolRegistry.cpp
        ../src/agent/ToolRegistry.h
        ../src/agent/AgentTool.h
)

target_include_directories(
        tool_registry_tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(
        tool_registry_tests
        PRIVATE
        Drogon::Drogon
)

target_compile_features(
        tool_registry_tests
        PRIVATE
        cxx_std_20
)

if(MSVC)
    target_compile_options(
            tool_registry_tests
            PRIVATE
            /W4
            /permissive-
    )
else()
    target_compile_options(
            tool_registry_tests
            PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

add_test(
        NAME tool_registry_tests
        COMMAND tool_registry_tests
)


add_executable(
        gemini_conversation_tests
        GeminiConversationTests.cpp
        ../src/ai/GeminiConversation.cpp
        ../src/ai/GeminiConversation.h
)

target_include_directories(
        gemini_conversation_tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(
        gemini_conversation_tests
        PRIVATE
        Drogon::Drogon
)

target_compile_features(
        gemini_conversation_tests
        PRIVATE
        cxx_std_20
)

if(MSVC)
    target_compile_options(
            gemini_conversation_tests
            PRIVATE
            /W4
            /permissive-
    )
else()
    target_compile_options(
            gemini_conversation_tests
            PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

add_test(
        NAME gemini_conversation_tests
        COMMAND gemini_conversation_tests
)