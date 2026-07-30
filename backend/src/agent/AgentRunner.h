#pragma once

#include "agent/ToolRegistry.h"
#include "ai/GeminiClient.h"
#include "ai/GeminiConversation.h"
#include "ai/GeminiResponse.h"

#include <json/json.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

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
        struct RunState
        {
            Json::Value contents{
                Json::arrayValue
            };

            Json::Value tools{
                Json::arrayValue
            };

            Json::Value steps{
                Json::arrayValue
            };

            std::size_t completedSteps{};

            std::shared_ptr<SuccessCallback>
                successCallback;

            std::shared_ptr<ErrorCallback>
                errorCallback;
        };

        void continueRun(
            const std::shared_ptr<RunState> &state
        ) const;

        void handleGeminiResponse(
            const std::shared_ptr<RunState> &state,
            healthcare::ai::GeminiResponse response
        ) const;

        void executeFunctionCalls(
            const std::shared_ptr<RunState> &state,
            const std::shared_ptr<
                std::vector<
                    healthcare::ai::GeminiFunctionCall
                >
            > &functionCalls,
            const std::shared_ptr<
                std::vector<
                    healthcare::ai::GeminiFunctionResult
                >
            > &functionResults,
            std::size_t functionIndex
        ) const;

        std::shared_ptr<
            healthcare::ai::GeminiClient
        > geminiClient_;

        std::shared_ptr<ToolRegistry> toolRegistry_;

        std::size_t maximumSteps_;
    };
}