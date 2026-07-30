#pragma once

#include <json/json.h>

#include <string>

namespace healthcare::ai
{
    class GeminiConversation final
    {
    public:
        [[nodiscard]]
        static Json::Value create(
            const std::string &prompt
        );

        static void appendModelContent(
            Json::Value &contents,
            const Json::Value &modelContent
        );

        static void appendFunctionResponse(
            Json::Value &contents,
            const std::string &functionName,
            const Json::Value &functionResult,
            const std::string &functionCallId = ""
        );
    };
}