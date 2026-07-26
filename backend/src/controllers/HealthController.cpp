#include "controllers/HealthController.h"

#include <json/json.h>

void HealthController::asyncHandleHttpRequest(
    const drogon::HttpRequestPtr &,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback
)
{
    Json::Value responseBody;
    responseBody["status"] = "ok";
    responseBody["service"] = "healthcare-ai-agent";
    responseBody["version"] = "0.1.0";

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseBody);

    response->setStatusCode(drogon::k200OK);

    callback(response);
}