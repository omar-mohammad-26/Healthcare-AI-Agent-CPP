#pragma once

#include <json/json.h>

#include <optional>
#include <string>
#include <vector>

namespace healthcare::ai
{
    enum class GeminiResponseType
    {
        Text,
        FunctionCall
    };

    struct GeminiFunctionCall
    {
        std::string id;
        std::string name;

        Json::Value arguments{
            Json::objectValue
        };
    };

    struct GeminiResponse
    {
        GeminiResponseType type{
            GeminiResponseType::Text
        };

        std::string text;

        GeminiFunctionCall functionCall;

        std::vector<GeminiFunctionCall>
            functionCalls;

        Json::Value modelContent{
            Json::objectValue
        };
    };

    class GeminiResponseParser final
    {
    public:
        [[nodiscard]]
        static std::optional<GeminiResponse> parse(
            const Json::Value &responseBody
        );
    };
}