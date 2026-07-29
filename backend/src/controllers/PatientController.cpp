#include "controllers/PatientController.h"

#include "models/Patient.h"
#include "services/PatientService.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <functional>
#include <memory>
#include <optional>
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
    const std::string &field = ""
)
{
    Json::Value responseBody;
    responseBody["status"] = "error";
    responseBody["code"] = code;
    responseBody["message"] = message;

    if (!field.empty())
    {
        responseBody["field"] = field;
    }

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(
            responseBody
        );

    response->setStatusCode(statusCode);

    (*callback)(response);
}
}

void PatientController::listPatients(
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

            sendError(
                sharedCallback,
                drogon::k500InternalServerError,
                "patient_query_failed",
                "Unable to retrieve patients."
            );
        }
    );
}

void PatientController::createPatient(
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

    if (!requestBody->isMember("full_name") ||
        !(*requestBody)["full_name"].isString())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_request_body",
            "Full name is required and must be a string.",
            "full_name"
        );

        return;
    }

    if (!requestBody->isMember("phone") ||
        !(*requestBody)["phone"].isString())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_request_body",
            "Phone is required and must be a string.",
            "phone"
        );

        return;
    }

    if (!requestBody->isMember("age") ||
        !(*requestBody)["age"].isInt())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_request_body",
            "Age is required and must be an integer.",
            "age"
        );

        return;
    }

    if (!requestBody->isMember("gender") ||
        !(*requestBody)["gender"].isString())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_request_body",
            "Gender is required and must be a string.",
            "gender"
        );

        return;
    }

    std::optional<std::string> medicalHistory;

    if (requestBody->isMember("medical_history") &&
        !(*requestBody)["medical_history"].isNull())
    {
        if (!(*requestBody)["medical_history"].isString())
        {
            sendError(
                sharedCallback,
                drogon::k400BadRequest,
                "invalid_request_body",
                "Medical history must be a string or null.",
                "medical_history"
            );

            return;
        }

        medicalHistory =
            (*requestBody)["medical_history"]
                .asString();
    }

    healthcare::models::CreatePatientInput input;

    input.fullName =
        (*requestBody)["full_name"].asString();

    input.phone =
        (*requestBody)["phone"].asString();

    input.age =
        (*requestBody)["age"].asInt();

    input.gender =
        (*requestBody)["gender"].asString();

    input.medicalHistory =
        std::move(medicalHistory);

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

    healthcare::services::PatientService
        patientService(databaseClient);

    patientService.createPatient(
        std::move(input),
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
                drogon::k201Created
            );

            (*sharedCallback)(response);
        },
        [
            sharedCallback
        ](
            healthcare::services::
                PatientServiceError error
        )
        {
            switch (error.type)
            {
                case healthcare::services::
                    PatientServiceErrorType::Validation:
                    sendError(
                        sharedCallback,
                        drogon::k400BadRequest,
                        "invalid_patient_data",
                        error.message,
                        error.field
                    );
                    return;

                case healthcare::services::
                    PatientServiceErrorType::Conflict:
                    sendError(
                        sharedCallback,
                        drogon::k409Conflict,
                        "patient_phone_conflict",
                        error.message,
                        error.field
                    );
                    return;

                case healthcare::services::
                    PatientServiceErrorType::Database:
                    LOG_ERROR
                        << "Failed to create patient: "
                        << error.message;

                    sendError(
                        sharedCallback,
                        drogon::k500InternalServerError,
                        "patient_creation_failed",
                        "Unable to create the patient."
                    );
                    return;
            }
        }
    );
}