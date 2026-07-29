#include "controllers/GeminiController.h"

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
            switch (error.type)
            {
                case healthcare::ai::
                    GeminiErrorType::Configuration:
                    sendError(
                        sharedCallback,
                        drogon::k503ServiceUnavailable,
                        "gemini_not_configured",
                        error.message
                    );
                    return;

                case healthcare::ai::
                    GeminiErrorType::Network:
                    sendError(
                        sharedCallback,
                        drogon::k502BadGateway,
                        "gemini_network_error",
                        error.message
                    );
                    return;

                case healthcare::ai::
                    GeminiErrorType::Api:
                    sendError(
                        sharedCallback,
                        drogon::k502BadGateway,
                        "gemini_api_error",
                        error.message,
                        error.statusCode
                    );
                    return;

                case healthcare::ai::
                    GeminiErrorType::InvalidResponse:
                    sendError(
                        sharedCallback,
                        drogon::k502BadGateway,
                        "gemini_invalid_response",
                        error.message,
                        error.statusCode
                    );
                    return;
            }
        }
    );
}