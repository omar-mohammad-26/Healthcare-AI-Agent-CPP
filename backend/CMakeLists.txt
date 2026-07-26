find_package(Drogon CONFIG REQUIRED)

set(BACKEND_SOURCES
        src/main.cpp

        src/config/DatabaseConfig.cpp
        src/config/DatabaseConfig.h

        src/controllers/HealthController.cpp
        src/controllers/HealthController.h

        src/controllers/DatabaseHealthController.cpp
        src/controllers/DatabaseHealthController.h
)

add_executable(
        healthcare_ai_agent
        ${BACKEND_SOURCES}
)

target_include_directories(
        healthcare_ai_agent
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
        healthcare_ai_agent
        PRIVATE
        Drogon::Drogon
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

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/config.json
        ${CMAKE_CURRENT_BINARY_DIR}/config.json
        COPYONLY
)