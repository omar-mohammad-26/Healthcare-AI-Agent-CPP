#include "controllers/DatabaseHealthController.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <functional>
#include <memory>
#include <utility>

void DatabaseHealthController::asyncHandleHttpRequest(
    const drogon::HttpRequestPtr &,
    std::function<
        void(const drogon::HttpResponsePtr &)
    > &&callback
)
{
    using ResponseCallback =
        std::function<
            void(const drogon::HttpResponsePtr &)
        >;

    auto sharedCallback =
        std::make_shared<ResponseCallback>(
            std::move(callback)
        );

    const auto databaseClient =
        drogon::app().getDbClient("default");

    if (!databaseClient)
    {
        Json::Value responseBody;
        responseBody["status"] = "error";
        responseBody["code"] = "database_client_unavailable";
        responseBody["message"] =
            "The PostgreSQL client is not configured.";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(
                responseBody
            );

        response->setStatusCode(
            drogon::k503ServiceUnavailable
        );

        (*sharedCallback)(response);
        return;
    }

    databaseClient->execSqlAsync(
        "SELECT "
        "current_database() AS database_name, "
        "current_user AS database_user",
        [sharedCallback](
            const drogon::orm::Result &result
        )
        {
            if (result.size() == 0)
            {
                Json::Value responseBody;
                responseBody["status"] = "error";
                responseBody["code"] =
                    "database_empty_response";
                responseBody["message"] =
                    "PostgreSQL returned no health-check data.";

                auto response =
                    drogon::HttpResponse::newHttpJsonResponse(
                        responseBody
                    );

                response->setStatusCode(
                    drogon::k503ServiceUnavailable
                );

                (*sharedCallback)(response);
                return;
            }

            const auto row = result[0];

            Json::Value responseBody;
            responseBody["status"] = "ok";
            responseBody["service"] = "postgresql";

            responseBody["database"] =
                row["database_name"].as<std::string>();

            responseBody["user"] =
                row["database_user"].as<std::string>();

            auto response =
                drogon::HttpResponse::newHttpJsonResponse(
                    responseBody
                );

            response->setStatusCode(drogon::k200OK);

            (*sharedCallback)(response);
        },
        [sharedCallback](
            const drogon::orm::DrogonDbException &exception
        )
        {
            LOG_ERROR
                << "PostgreSQL health check failed: "
                << exception.base().what();

            Json::Value responseBody;
            responseBody["status"] = "error";
            responseBody["code"] =
                "database_connection_failed";
            responseBody["message"] =
                "The PostgreSQL database is unavailable.";

            auto response =
                drogon::HttpResponse::newHttpJsonResponse(
                    responseBody
                );

            response->setStatusCode(
                drogon::k503ServiceUnavailable
            );

            (*sharedCallback)(response);
        }
    );
}