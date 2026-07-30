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

    geminiClient->generateConversation(
        contents,
        tools,
        [
            sharedSuccessCallback,
            sharedErrorCallback,
            geminiClient
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

            (*sharedErrorCallback)(
                {
                    AgentRunnerErrorType::Tool,
                    "Gemini requested the tool '" +
                    response.functionCall.name +
                    "', but Agent tool execution is not implemented yet."
                }
            );
        },
        [
            sharedErrorCallback,
            geminiClient
        ](
            healthcare::ai::GeminiError error
        )
        {
            static_cast<void>(geminiClient);

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