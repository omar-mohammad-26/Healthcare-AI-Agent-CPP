#include "controllers/GeminiController.h"

#include "agent/AgentRunner.h"
#include "agent/ToolRegistryFactory.h"
#include "ai/GeminiClient.h"
#include "config/GeminiConfig.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace
{
using ResponseCallback =
    std::function<
        void(const drogon::HttpResponsePtr &)
    >;

using SharedResponseCallback =
    std::shared_ptr<ResponseCallback>;

void sendError(
    const SharedResponseCallback &callback,
    const drogon::HttpStatusCode statusCode,
    const std::string &code,
    const std::string &message,
    const int providerStatusCode = 0
)
{
    Json::Value responseBody;

    responseBody["status"] =
        "error";

    responseBody["provider"] =
        "gemini";

    responseBody["code"] =
        code;

    responseBody["message"] =
        message;

    if (providerStatusCode > 0)
    {
        responseBody["provider_status_code"] =
            providerStatusCode;
    }

    auto response =
        drogon::HttpResponse::
            newHttpJsonResponse(
                responseBody
            );

    response->setStatusCode(
        statusCode
    );

    (*callback)(response);
}

void sendGeminiError(
    const SharedResponseCallback &callback,
    healthcare::ai::GeminiError error
)
{
    switch (error.type)
    {
        case healthcare::ai::
            GeminiErrorType::Configuration:
            sendError(
                callback,
                drogon::k503ServiceUnavailable,
                "gemini_not_configured",
                error.message
            );
            return;

        case healthcare::ai::
            GeminiErrorType::Network:
            sendError(
                callback,
                drogon::k502BadGateway,
                "gemini_network_error",
                error.message
            );
            return;

        case healthcare::ai::
            GeminiErrorType::Api:
            sendError(
                callback,
                drogon::k502BadGateway,
                "gemini_api_error",
                error.message,
                error.statusCode
            );
            return;

        case healthcare::ai::
            GeminiErrorType::InvalidResponse:
            sendError(
                callback,
                drogon::k502BadGateway,
                "gemini_invalid_response",
                error.message,
                error.statusCode
            );
            return;
    }
}

void sendAgentError(
    const SharedResponseCallback &callback,
    healthcare::agent::AgentRunnerError error
)
{
    switch (error.type)
    {
        case healthcare::agent::
            AgentRunnerErrorType::Validation:
            sendError(
                callback,
                drogon::k400BadRequest,
                "invalid_agent_prompt",
                error.message
            );
            return;

        case healthcare::agent::
            AgentRunnerErrorType::Gemini:
            sendError(
                callback,
                drogon::k502BadGateway,
                "agent_gemini_error",
                error.message
            );
            return;

        case healthcare::agent::
            AgentRunnerErrorType::Tool:
            sendError(
                callback,
                drogon::k500InternalServerError,
                "agent_tool_error",
                error.message
            );
            return;

        case healthcare::agent::
            AgentRunnerErrorType::StepLimit:
            sendError(
                callback,
                drogon::k500InternalServerError,
                "agent_step_limit_reached",
                error.message
            );
            return;
    }
}
}

void GeminiController::checkConnection(
    const drogon::HttpRequestPtr &,
    std::function<
        void(const drogon::HttpResponsePtr &)
    > &&callback
) const
{
    auto sharedCallback =
        std::make_shared<ResponseCallback>(
            std::move(callback)
        );

    const healthcare::config::GeminiConfig
        configuration =
            healthcare::config::GeminiConfig::
                fromEnvironment();

    const auto geminiClient =
        std::make_shared<
            healthcare::ai::GeminiClient
        >(
            configuration
        );

    geminiClient->generateText(
        "Reply with exactly GEMINI_OK and nothing else.",
        [
            sharedCallback,
            geminiClient,
            configuration
        ](
            std::string generatedText
        )
        {
            Json::Value responseBody;

            responseBody["status"] =
                "ok";

            responseBody["provider"] =
                "gemini";

            responseBody["model"] =
                configuration.model;

            responseBody["response"] =
                std::move(generatedText);

            auto response =
                drogon::HttpResponse::
                    newHttpJsonResponse(
                        responseBody
                    );

            response->setStatusCode(
                drogon::k200OK
            );

            (*sharedCallback)(response);
        },
        [
            sharedCallback,
            geminiClient
        ](
            healthcare::ai::GeminiError error
        )
        {
            sendGeminiError(
                sharedCallback,
                std::move(error)
            );
        }
    );
}

void GeminiController::previewToolCall(
    const drogon::HttpRequestPtr &request,
    std::function<
        void(const drogon::HttpResponsePtr &)
    > &&callback
) const
{
    auto sharedCallback =
        std::make_shared<ResponseCallback>(
            std::move(callback)
        );

    const auto requestBody =
        request->getJsonObject();

    if (!requestBody ||
        !requestBody->isObject())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_request_body",
            "Request body must be a valid JSON object."
        );

        return;
    }

    if (!requestBody->isMember("prompt") ||
        !(*requestBody)["prompt"].isString() ||
        (*requestBody)["prompt"].asString().empty())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_prompt",
            "The prompt field must be a non-empty string."
        );

        return;
    }

    const auto databaseClient =
        drogon::app().getDbClient(
            "default"
        );

    if (!databaseClient)
    {
        sendError(
            sharedCallback,
            drogon::k503ServiceUnavailable,
            "database_client_unavailable",
            "The PostgreSQL client is not configured."
        );

        return;
    }

    const auto toolRegistry =
        healthcare::agent::
            ToolRegistryFactory::create(
                databaseClient
            );

    const Json::Value tools =
        toolRegistry->geminiTools();

    const healthcare::config::GeminiConfig
        configuration =
            healthcare::config::GeminiConfig::
                fromEnvironment();

    const auto geminiClient =
        std::make_shared<
            healthcare::ai::GeminiClient
        >(
            configuration
        );

    const std::string prompt =
        (*requestBody)["prompt"].asString();

    geminiClient->generateWithTools(
        prompt,
        tools,
        [
            sharedCallback,
            geminiClient,
            toolRegistry,
            configuration
        ](
            healthcare::ai::GeminiResponse
                geminiResponse
        )
        {
            Json::Value responseBody;

            responseBody["status"] =
                "success";

            responseBody["provider"] =
                "gemini";

            responseBody["model"] =
                configuration.model;

            if (geminiResponse.type ==
                healthcare::ai::
                    GeminiResponseType::FunctionCall)
            {
                responseBody["type"] =
                    "function_call";

                Json::Value functionCall;

                functionCall["name"] =
                    geminiResponse.functionCall.name;

                functionCall["arguments"] =
                    geminiResponse.functionCall.arguments;

                if (!geminiResponse
                         .functionCall
                         .id
                         .empty())
                {
                    functionCall["id"] =
                        geminiResponse.functionCall.id;
                }

                responseBody["function_call"] =
                    std::move(functionCall);
            }
            else
            {
                responseBody["type"] =
                    "text";

                responseBody["text"] =
                    std::move(
                        geminiResponse.text
                    );
            }

            auto response =
                drogon::HttpResponse::
                    newHttpJsonResponse(
                        responseBody
                    );

            response->setStatusCode(
                drogon::k200OK
            );

            (*sharedCallback)(response);
        },
        [
            sharedCallback,
            geminiClient,
            toolRegistry
        ](
            healthcare::ai::GeminiError error
        )
        {
            sendGeminiError(
                sharedCallback,
                std::move(error)
            );
        }
    );
}

void GeminiController::runAgent(
    const drogon::HttpRequestPtr &request,
    std::function<
        void(const drogon::HttpResponsePtr &)
    > &&callback
) const
{
    auto sharedCallback =
        std::make_shared<ResponseCallback>(
            std::move(callback)
        );

    const auto requestBody =
        request->getJsonObject();

    if (!requestBody ||
        !requestBody->isObject())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_request_body",
            "Request body must be a valid JSON object."
        );

        return;
    }

    if (!requestBody->isMember("prompt") ||
        !(*requestBody)["prompt"].isString() ||
        (*requestBody)["prompt"].asString().empty())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_prompt",
            "The prompt field must be a non-empty string."
        );

        return;
    }

    const auto databaseClient =
        drogon::app().getDbClient(
            "default"
        );

    if (!databaseClient)
    {
        sendError(
            sharedCallback,
            drogon::k503ServiceUnavailable,
            "database_client_unavailable",
            "The PostgreSQL client is not configured."
        );

        return;
    }

    const healthcare::config::GeminiConfig
        configuration =
            healthcare::config::GeminiConfig::
                fromEnvironment();

    const auto geminiClient =
        std::make_shared<
            healthcare::ai::GeminiClient
        >(
            configuration
        );

    const auto toolRegistry =
        healthcare::agent::
            ToolRegistryFactory::create(
                databaseClient
            );

    const auto agentRunner =
        std::make_shared<
            healthcare::agent::AgentRunner
        >(
            geminiClient,
            toolRegistry
        );

    const std::string prompt =
        (*requestBody)["prompt"].asString();

    agentRunner->run(
        prompt,
        [
            sharedCallback,
            agentRunner,
            configuration
        ](
            healthcare::agent::AgentRunResult result
        )
        {
            Json::Value responseBody;

            responseBody["status"] =
                "success";

            responseBody["provider"] =
                "gemini";

            responseBody["model"] =
                configuration.model;

            responseBody["answer"] =
                std::move(result.answer);

            responseBody["steps"] =
                std::move(result.steps);

            responseBody["step_count"] =
                static_cast<Json::UInt64>(
                    responseBody["steps"].size()
                );

            auto response =
                drogon::HttpResponse::
                    newHttpJsonResponse(
                        responseBody
                    );

            response->setStatusCode(
                drogon::k200OK
            );

            (*sharedCallback)(response);
        },
        [
            sharedCallback,
            agentRunner
        ](
            healthcare::agent::AgentRunnerError error
        )
        {
            sendAgentError(
                sharedCallback,
                std::move(error)
            );
        }
    );
}