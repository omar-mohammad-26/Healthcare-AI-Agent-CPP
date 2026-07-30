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

    const auto tools =
        std::make_shared<Json::Value>(
            toolRegistry_->geminiTools()
        );

    if (!tools->isArray() ||
        tools->empty())
    {
        (*sharedErrorCallback)(
            {
                AgentRunnerErrorType::Tool,
                "The Agent tool registry contains no Gemini tools."
            }
        );

        return;
    }

    const auto contents =
        std::make_shared<Json::Value>(
            healthcare::ai::
                GeminiConversation::create(
                    prompt
                )
        );

    const auto steps =
        std::make_shared<Json::Value>(
            Json::arrayValue
        );

    const auto geminiClient =
        geminiClient_;

    const auto toolRegistry =
        toolRegistry_;

    geminiClient->generateConversation(
        *contents,
        *tools,
        [
            sharedSuccessCallback,
            sharedErrorCallback,
            geminiClient,
            toolRegistry,
            contents,
            tools,
            steps
        ](
            healthcare::ai::GeminiResponse response
        )
        {
            if (response.type ==
                healthcare::ai::
                    GeminiResponseType::Text)
            {
                AgentRunResult result;

                result.answer =
                    std::move(response.text);

                result.steps =
                    *steps;

                (*sharedSuccessCallback)(
                    std::move(result)
                );

                return;
            }

            healthcare::ai::
                GeminiConversation::appendModelContent(
                    *contents,
                    response.modelContent
                );

            const std::string functionName =
                response.functionCall.name;

            const Json::Value functionArguments =
                response.functionCall.arguments;

            const std::string functionCallId =
                response.functionCall.id;

            toolRegistry->execute(
                functionName,
                functionArguments,
                [
                    sharedSuccessCallback,
                    sharedErrorCallback,
                    geminiClient,
                    toolRegistry,
                    contents,
                    tools,
                    steps,
                    functionName,
                    functionArguments,
                    functionCallId
                ](
                    Json::Value toolResult
                )
                {
                    Json::Value step;

                    step["type"] =
                        "tool";

                    step["name"] =
                        functionName;

                    step["arguments"] =
                        functionArguments;

                    step["result"] =
                        toolResult;

                    steps->append(
                        std::move(step)
                    );

                    healthcare::ai::
                        GeminiConversation::
                            appendFunctionResponse(
                                *contents,
                                functionName,
                                toolResult,
                                functionCallId
                            );

                    geminiClient->generateConversation(
                        *contents,
                        *tools,
                        [
                            sharedSuccessCallback,
                            sharedErrorCallback,
                            geminiClient,
                            toolRegistry,
                            contents,
                            tools,
                            steps
                        ](
                            healthcare::ai::
                                GeminiResponse finalResponse
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

                            static_cast<void>(
                                contents
                            );

                            static_cast<void>(
                                tools
                            );

                            if (finalResponse.type ==
                                healthcare::ai::
                                    GeminiResponseType::Text)
                            {
                                AgentRunResult result;

                                result.answer =
                                    std::move(
                                        finalResponse.text
                                    );

                                result.steps =
                                    *steps;

                                (*sharedSuccessCallback)(
                                    std::move(result)
                                );

                                return;
                            }

                            (*sharedErrorCallback)(
                                {
                                    AgentRunnerErrorType::Tool,
                                    "Gemini requested another tool, but multi-step execution is not implemented yet."
                                }
                            );
                        },
                        [
                            sharedErrorCallback,
                            geminiClient,
                            toolRegistry,
                            contents,
                            tools,
                            steps
                        ](
                            healthcare::ai::
                                GeminiError error
                        )
                        {
                            static_cast<void>(
                                geminiClient
                            );

                            static_cast<void>(
                                toolRegistry
                            );

                            static_cast<void>(
                                contents
                            );

                            static_cast<void>(
                                tools
                            );

                            static_cast<void>(
                                steps
                            );

                            (*sharedErrorCallback)(
                                {
                                    AgentRunnerErrorType::Gemini,
                                    std::move(error.message)
                                }
                            );
                        }
                    );
                },
                [
                    sharedErrorCallback,
                    geminiClient,
                    toolRegistry,
                    contents,
                    tools,
                    steps,
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

                    static_cast<void>(
                        contents
                    );

                    static_cast<void>(
                        tools
                    );

                    static_cast<void>(
                        steps
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
            toolRegistry,
            contents,
            tools,
            steps
        ](
            healthcare::ai::GeminiError error
        )
        {
            static_cast<void>(
                geminiClient
            );

            static_cast<void>(
                toolRegistry
            );

            static_cast<void>(
                contents
            );

            static_cast<void>(
                tools
            );

            static_cast<void>(
                steps
            );

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