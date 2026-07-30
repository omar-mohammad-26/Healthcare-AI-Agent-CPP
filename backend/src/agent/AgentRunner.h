#pragma once

#include "agent/ToolRegistry.h"
#include "ai/GeminiClient.h"

#include <json/json.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

namespace healthcare::agent
{
    enum class AgentRunnerErrorType
    {
        Validation,
        Gemini,
        Tool,
        StepLimit
    };

    struct AgentRunnerError
    {
        AgentRunnerErrorType type;
        std::string message;
    };

    struct AgentRunResult
    {
        std::string answer;

        Json::Value steps{
            Json::arrayValue
        };
    };

    class AgentRunner final
    {
    public:
        using SuccessCallback =
            std::function<void(AgentRunResult)>;

        using ErrorCallback =
            std::function<void(AgentRunnerError)>;

        AgentRunner(
            std::shared_ptr<
                healthcare::ai::GeminiClient
            > geminiClient,
            std::shared_ptr<ToolRegistry> toolRegistry,
            std::size_t maximumSteps = 5
        );

        void run(
            const std::string &prompt,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        std::shared_ptr<
            healthcare::ai::GeminiClient
        > geminiClient_;

        std::shared_ptr<ToolRegistry> toolRegistry_;

        std::size_t maximumSteps_;
    };
}