add_executable(
    healthcare_ai_agent
    src/main.cc
)

target_compile_features(
    healthcare_ai_agent
    PRIVATE
    cxx_std_20
)

if(MSVC)
    target_compile_options(
        healthcare_ai_agent
        PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(
        healthcare_ai_agent
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
