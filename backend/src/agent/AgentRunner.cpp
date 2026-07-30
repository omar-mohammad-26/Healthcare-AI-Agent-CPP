#include "agent/AgentRunner.h"

#include "ai/GeminiConversation.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

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

    Json::Value tools =
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

    auto state =
        std::make_shared<RunState>();

    state->contents =
        healthcare::ai::
            GeminiConversation::create(
                prompt
            );

    state->tools =
        std::move(tools);

    state->successCallback =
        std::move(sharedSuccessCallback);

    state->errorCallback =
        std::move(sharedErrorCallback);

    continueRun(state);
}

void AgentRunner::continueRun(
    const std::shared_ptr<RunState> &state
) const
{
    if (!state)
    {
        return;
    }

    geminiClient_->generateConversation(
        state->contents,
        state->tools,
        [
            this,
            state
        ](
            healthcare::ai::GeminiResponse response
        )
        {
            handleGeminiResponse(
                state,
                std::move(response)
            );
        },
        [
            state
        ](
            healthcare::ai::GeminiError error
        )
        {
            (*state->errorCallback)(
                {
                    AgentRunnerErrorType::Gemini,
                    std::move(error.message)
                }
            );
        }
    );
}

void AgentRunner::handleGeminiResponse(
    const std::shared_ptr<RunState> &state,
    healthcare::ai::GeminiResponse response
) const
{
    if (!state)
    {
        return;
    }

    if (response.type ==
        healthcare::ai::
            GeminiResponseType::Text)
    {
        AgentRunResult result;

        result.answer =
            std::move(response.text);

        result.steps =
            state->steps;

        (*state->successCallback)(
            std::move(result)
        );

        return;
    }

    auto functionCalls =
        std::make_shared<
            std::vector<
                healthcare::ai::GeminiFunctionCall
            >
        >(
            std::move(response.functionCalls)
        );

    if (functionCalls->empty() &&
        !response.functionCall.name.empty())
    {
        functionCalls->push_back(
            std::move(response.functionCall)
        );
    }

    if (functionCalls->empty())
    {
        (*state->errorCallback)(
            {
                AgentRunnerErrorType::Gemini,
                "Gemini returned an empty function-call response."
            }
        );

        return;
    }

    if (state->completedSteps +
            functionCalls->size() >
        maximumSteps_)
    {
        (*state->errorCallback)(
            {
                AgentRunnerErrorType::StepLimit,
                "The Agent reached the maximum number of tool steps."
            }
        );

        return;
    }

    healthcare::ai::
        GeminiConversation::appendModelContent(
            state->contents,
            response.modelContent
        );

    auto functionResults =
        std::make_shared<
            std::vector<
                healthcare::ai::GeminiFunctionResult
            >
        >();

    functionResults->reserve(
        functionCalls->size()
    );

    executeFunctionCalls(
        state,
        functionCalls,
        functionResults,
        0
    );
}

void AgentRunner::executeFunctionCalls(
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
    const std::size_t functionIndex
) const
{
    if (!state ||
        !functionCalls ||
        !functionResults)
    {
        return;
    }

    if (functionIndex >=
        functionCalls->size())
    {
        healthcare::ai::
            GeminiConversation::
                appendFunctionResponses(
                    state->contents,
                    *functionResults
                );

        continueRun(state);

        return;
    }

    const healthcare::ai::GeminiFunctionCall
        functionCall =
            (*functionCalls)[functionIndex];

    const std::string functionName =
        functionCall.name;

    const Json::Value functionArguments =
        functionCall.arguments;

    const std::string functionCallId =
        functionCall.id;

    toolRegistry_->execute(
        functionName,
        functionArguments,
        [
            this,
            state,
            functionCalls,
            functionResults,
            functionIndex,
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

            step["number"] =
                static_cast<Json::UInt64>(
                    state->completedSteps + 1
                );

            step["name"] =
                functionName;

            step["arguments"] =
                functionArguments;

            step["result"] =
                toolResult;

            state->steps.append(
                std::move(step)
            );

            ++state->completedSteps;

            healthcare::ai::GeminiFunctionResult
                functionResult;

            functionResult.functionCallId =
                functionCallId;

            functionResult.functionName =
                functionName;

            functionResult.functionResult =
                std::move(toolResult);

            functionResults->push_back(
                std::move(functionResult)
            );

            executeFunctionCalls(
                state,
                functionCalls,
                functionResults,
                functionIndex + 1
            );
        },
        [
            state,
            functionName
        ](
            AgentToolError error
        )
        {
            (*state->errorCallback)(
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
}
}