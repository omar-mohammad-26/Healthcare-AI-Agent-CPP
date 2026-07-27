#include "controllers/DepartmentController.h"

#include "services/DepartmentService.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <functional>
#include <memory>
#include <string>
#include <utility>

void DepartmentController::listDepartments(
    const drogon::HttpRequestPtr &,
    std::function<
        void(const drogon::HttpResponsePtr &)
    > &&callback
) const
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
        responseBody["code"] =
            "database_client_unavailable";
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

    healthcare::services::DepartmentService
        departmentService(databaseClient);

    departmentService.listDepartments(
        [
            sharedCallback
        ](
            Json::Value responseBody
        )
        {
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
            sharedCallback
        ](
            const std::string &errorMessage
        )
        {
            LOG_ERROR
                << "Failed to list departments: "
                << errorMessage;

            Json::Value responseBody;
            responseBody["status"] = "error";
            responseBody["code"] =
                "department_query_failed";
            responseBody["message"] =
                "Unable to retrieve departments.";

            auto response =
                drogon::HttpResponse::
                    newHttpJsonResponse(
                        responseBody
                    );

            response->setStatusCode(
                drogon::k500InternalServerError
            );

            (*sharedCallback)(response);
        }
    );
}