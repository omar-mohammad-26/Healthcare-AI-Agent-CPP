#include "config/GeminiConfig.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

namespace
{
std::string trim(
    const std::string &value
)
{
    const auto firstCharacter =
        std::find_if_not(
            value.begin(),
            value.end(),
            [](
                const unsigned char character
            )
            {
                return std::isspace(character);
            }
        );

    if (firstCharacter == value.end())
    {
        return "";
    }

    const auto lastCharacter =
        std::find_if_not(
            value.rbegin(),
            value.rend(),
            [](
                const unsigned char character
            )
            {
                return std::isspace(character);
            }
        ).base();

    return std::string(
        firstCharacter,
        lastCharacter
    );
}

std::string readEnvironmentVariable(
    const char *variableName
)
{
    const char *environmentValue =
        std::getenv(variableName);

    if (environmentValue == nullptr)
    {
        return "";
    }

    return trim(environmentValue);
}

std::string normalizeModelName(
    std::string model
)
{
    constexpr const char *modelPrefix =
        "models/";

    if (model.starts_with(modelPrefix))
    {
        model.erase(
            0,
            std::char_traits<char>::length(
                modelPrefix
            )
        );
    }

    return model;
}

std::string normalizeBaseUrl(
    std::string baseUrl
)
{
    while (!baseUrl.empty() &&
           baseUrl.back() == '/')
    {
        baseUrl.pop_back();
    }

    return baseUrl;
}
}

namespace healthcare::config
{
bool GeminiConfig::isConfigured() const
{
    return
        !apiKey.empty() &&
        !model.empty() &&
        !baseUrl.empty();
}

GeminiConfig GeminiConfig::fromEnvironment()
{
    GeminiConfig configuration;

    configuration.apiKey =
        readEnvironmentVariable(
            "GEMINI_API_KEY"
        );

    if (configuration.apiKey.empty())
    {
        configuration.apiKey =
            readEnvironmentVariable(
                "GOOGLE_API_KEY"
            );
    }

    configuration.model =
        normalizeModelName(
            readEnvironmentVariable(
                "GEMINI_MODEL"
            )
        );

    if (configuration.model.empty())
    {
        configuration.model =
            "gemini-3.5-flash";
    }

    configuration.baseUrl =
        normalizeBaseUrl(
            readEnvironmentVariable(
                "GEMINI_BASE_URL"
            )
        );

    if (configuration.baseUrl.empty())
    {
        configuration.baseUrl =
            "https://generativelanguage.googleapis.com";
    }

    return configuration;
}
}