#include "agent/AgentRunner.h"

#include "ai/GeminiConversation.h"
#include "ai/GeminiResponse.h"

#include <memory>
#include <string>
#include <utility>

namespace healthcare::agent
{
AgentRunner::AgentRunner(
    std::shared_ptr<
        healthcare::ai::GeminiClient
    > geminiClient,
    std::shared_ptr<ToolRegistry> toolRegistry,
    const std::size_t maximumSteps
)
    : geminiClient_(
          std::move(geminiClient)
      ),
      toolRegistry_(
          std::move(toolRegistry)
      ),
      maximumSteps_(
          maximumSteps
      )
{
}

void AgentRunner::run(
    const std::string &prompt,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    auto sharedSuccessCallback =
        std::make_shared<SuccessCallback>(
            std::move(successCallback)
        );

    auto sharedErrorCallback =
        std::make_shared<ErrorCallback>(
            std::move(errorCallback)
        );

    if (prompt.empty())
    {
        (*sharedErrorCallback)(
            {
                AgentRunnerErrorType::Validation,
                "The Agent prompt must not be empty."
            }
        );

        return;
    }

    if (!geminiClient_)
    {
        (*sharedErrorCallback)(
            {
                AgentRunnerErrorType::Gemini,
                "The Gemini client is not configured."
            }
        );

        return;
    }

    if (!toolRegistry_)
    {
        (*sharedErrorCallback)(
            {
                AgentRunnerErrorType::Tool,
                "The Agent tool registry is not configured."
            }
        );

        return;
    }

    if (maximumSteps_ == 0)
    {
        (*sharedErrorCallback)(
            {
                AgentRunnerErrorType::StepLimit,
                "The Agent maximum step count must be greater than zero."
            }
        );

        return;
    }

    const Json::Value tools =
        toolRegistry_->geminiTools();

    if (!tools.isArray() ||
        tools.empty())
    {
        (*sharedErrorCallback)(
            {
                AgentRunnerErrorType::Tool,
                "The Agent tool registry contains no Gemini tools."
            }
        );

        return;
    }

    const Json::Value contents =
        healthcare::ai::
            GeminiConversation::create(
                prompt
            );

    const auto geminiClient =
        geminiClient_;

    const auto toolRegistry =
        toolRegistry_;

    geminiClient->generateConversation(
        contents,
        tools,
        [
            sharedSuccessCallback,
            sharedErrorCallback,
            geminiClient,
            toolRegistry
        ](
            healthcare::ai::GeminiResponse response
        )
        {
            static_cast<void>(geminiClient);

            if (response.type ==
                healthcare::ai::
                    GeminiResponseType::Text)
            {
                AgentRunResult result;

                result.answer =
                    std::move(response.text);

                (*sharedSuccessCallback)(
                    std::move(result)
                );

                return;
            }

            const std::string functionName =
                response.functionCall.name;

            const Json::Value functionArguments =
                response.functionCall.arguments;

            toolRegistry->execute(
                functionName,
                functionArguments,
                [
                    sharedSuccessCallback,
                    sharedErrorCallback,
                    geminiClient,
                    toolRegistry,
                    functionName,
                    functionArguments
                ](
                    Json::Value toolResult
                )
                {
                    static_cast<void>(
                        sharedErrorCallback
                    );

                    static_cast<void>(
                        geminiClient
                    );

                    static_cast<void>(
                        toolRegistry
                    );

                    Json::Value step;

                    step["type"] =
                        "tool";

                    step["name"] =
                        functionName;

                    step["arguments"] =
                        functionArguments;

                    step["result"] =
                        std::move(toolResult);

                    AgentRunResult result;

                    result.answer =
                        "Executed the Agent tool '" +
                        functionName +
                        "' successfully.";

                    result.steps.append(
                        std::move(step)
                    );

                    (*sharedSuccessCallback)(
                        std::move(result)
                    );
                },
                [
                    sharedErrorCallback,
                    geminiClient,
                    toolRegistry,
                    functionName
                ](
                    AgentToolError error
                )
                {
                    static_cast<void>(
                        geminiClient
                    );

                    static_cast<void>(
                        toolRegistry
                    );

                    (*sharedErrorCallback)(
                        {
                            AgentRunnerErrorType::Tool,
                            "Agent tool '" +
                            functionName +
                            "' failed: " +
                            error.message
                        }
                    );
                }
            );
        },
        [
            sharedErrorCallback,
            geminiClient,
            toolRegistry
        ](
            healthcare::ai::GeminiError error
        )
        {
            static_cast<void>(geminiClient);
            static_cast<void>(toolRegistry);

            (*sharedErrorCallback)(
                {
                    AgentRunnerErrorType::Gemini,
                    std::move(error.message)
                }
            );
        }
    );
}
}