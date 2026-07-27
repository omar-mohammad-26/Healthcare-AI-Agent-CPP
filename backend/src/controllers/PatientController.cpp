#include "controllers/PatientController.h"

#include "services/PatientService.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <functional>
#include <memory>
#include <string>
#include <utility>

void PatientController::listPatients(
    const drogon::HttpRequestPtr &request,
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

    const std::string searchTerm =
        request->getParameter("search");

    healthcare::services::PatientService
        patientService(databaseClient);

    patientService.listPatients(
        searchTerm,
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
                << "Failed to list patients: "
                << errorMessage;

            Json::Value responseBody;
            responseBody["status"] = "error";
            responseBody["code"] =
                "patient_query_failed";
            responseBody["message"] =
                "Unable to retrieve patients.";

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