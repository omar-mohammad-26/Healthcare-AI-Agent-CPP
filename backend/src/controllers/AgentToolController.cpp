#include "controllers/AgentToolController.h"

#include "agent/ToolRegistry.h"
#include "agent/tools/SearchPatientTool.h"

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

std::shared_ptr<
    healthcare::agent::ToolRegistry
> createToolRegistry(
    const drogon::orm::DbClientPtr &databaseClient
)
{
    auto toolRegistry =
        std::make_shared<
            healthcare::agent::ToolRegistry
        >();

    toolRegistry->registerTool(
        std::make_shared<
            healthcare::agent::tools::
                SearchPatientTool
        >(
            databaseClient
        )
    );

    return toolRegistry;
}

void sendError(
    const SharedResponseCallback &callback,
    const drogon::HttpStatusCode statusCode,
    const std::string &code,
    const std::string &message,
    const std::string &field = ""
)
{
    Json::Value responseBody;

    responseBody["status"] =
        "error";

    responseBody["code"] =
        code;

    responseBody["message"] =
        message;

    if (!field.empty())
    {
        responseBody["field"] =
            field;
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
}

void AgentToolController::listTools(
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

    const auto databaseClient =
        drogon::app().getDbClient("default");

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
        createToolRegistry(databaseClient);

    Json::Value responseBody =
        toolRegistry->definitions();

    responseBody["status"] =
        "success";

    auto response =
        drogon::HttpResponse::
            newHttpJsonResponse(
                responseBody
            );

    response->setStatusCode(
        drogon::k200OK
    );

    (*sharedCallback)(response);
}

void AgentToolController::executeTool(
    const drogon::HttpRequestPtr &request,
    std::function<
        void(const drogon::HttpResponsePtr &)
    > &&callback,
    std::string toolName
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

    const auto databaseClient =
        drogon::app().getDbClient("default");

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
        createToolRegistry(databaseClient);

    toolRegistry->execute(
        toolName,
        *requestBody,
        [
            sharedCallback,
            toolRegistry
        ](
            Json::Value toolResult
        )
        {
            Json::Value responseBody;

            responseBody["status"] =
                "success";

            responseBody["data"] =
                std::move(toolResult);

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
            toolRegistry
        ](
            healthcare::agent::AgentToolError error
        )
        {
            switch (error.type)
            {
                case healthcare::agent::
                    AgentToolErrorType::Validation:
                    sendError(
                        sharedCallback,
                        drogon::k400BadRequest,
                        "invalid_tool_arguments",
                        error.message,
                        error.field
                    );
                    return;

                case healthcare::agent::
                    AgentToolErrorType::NotFound:
                    sendError(
                        sharedCallback,
                        drogon::k404NotFound,
                        "agent_tool_not_found",
                        error.message,
                        error.field
                    );
                    return;

                case healthcare::agent::
                    AgentToolErrorType::Database:
                    LOG_ERROR
                        << "Agent tool execution failed: "
                        << error.message;

                    sendError(
                        sharedCallback,
                        drogon::k500InternalServerError,
                        "agent_tool_execution_failed",
                        "Unable to execute the Agent tool."
                    );
                    return;
            }
        }
    );
}