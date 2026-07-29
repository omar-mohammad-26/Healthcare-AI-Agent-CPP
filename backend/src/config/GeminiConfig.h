#pragma once

#include <string>

namespace healthcare::config
{
    struct GeminiConfig
    {
        std::string apiKey;
        std::string model;
        std::string baseUrl;

        [[nodiscard]]
        bool isConfigured() const;

        [[nodiscard]]
        static GeminiConfig fromEnvironment();
    };
}