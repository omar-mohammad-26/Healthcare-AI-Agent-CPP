#include "controllers/DoctorController.h"

#include "services/DoctorService.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <functional>
#include <memory>
#include <string>
#include <utility>

void DoctorController::listDoctors(
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

    healthcare::services::DoctorService
        doctorService(databaseClient);

    doctorService.listDoctors(
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
                << "Failed to list doctors: "
                << errorMessage;

            Json::Value responseBody;
            responseBody["status"] = "error";
            responseBody["code"] =
                "doctor_query_failed";
            responseBody["message"] =
                "Unable to retrieve doctors.";

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