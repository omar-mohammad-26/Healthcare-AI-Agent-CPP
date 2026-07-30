#pragma once

#include <json/json.h>

#include <string>
#include <vector>

namespace healthcare::ai
{
    struct GeminiFunctionResult
    {
        std::string functionCallId;
        std::string functionName;

        Json::Value functionResult{
            Json::objectValue
        };
    };

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

        static void appendFunctionResponses(
            Json::Value &contents,
            const std::vector<GeminiFunctionResult>
                &functionResults
        );
    };
}