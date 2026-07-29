#include "controllers/AppointmentController.h"

#include "models/Appointment.h"
#include "services/AppointmentService.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <algorithm>
#include <cctype>
#include <exception>
#include <functional>
#include <limits>
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

std::string normalizeStatus(
    std::string status
)
{
    std::transform(
        status.begin(),
        status.end(),
        status.begin(),
        [](
            const unsigned char character
        )
        {
            return static_cast<char>(
                std::tolower(character)
            );
        }
    );

    return status;
}

bool isValidStatus(
    const std::string &status
)
{
    return status.empty() ||
           status == "scheduled" ||
           status == "cancelled" ||
           status == "completed";
}

bool parseOptionalPositiveInteger(
    const std::string &value,
    std::optional<int> &parsedValue
)
{
    if (value.empty())
    {
        parsedValue.reset();
        return true;
    }

    try
    {
        std::size_t parsedCharacters = 0;

        const long long number =
            std::stoll(
                value,
                &parsedCharacters
            );

        if (parsedCharacters != value.size() ||
            number <= 0 ||
            number >
                (std::numeric_limits<int>::max)())
        {
            return false;
        }

        parsedValue =
            static_cast<int>(number);

        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
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

void AppointmentController::listAppointments(
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

    const std::string status =
        normalizeStatus(
            request->getParameter("status")
        );

    if (!isValidStatus(status))
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_query_parameter",
            "Status must be scheduled, cancelled, or completed.",
            "status"
        );

        return;
    }

    std::optional<int> doctorId;

    if (!parseOptionalPositiveInteger(
            request->getParameter("doctor_id"),
            doctorId
        ))
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_query_parameter",
            "Doctor ID must be a positive integer.",
            "doctor_id"
        );

        return;
    }

    std::optional<int> patientId;

    if (!parseOptionalPositiveInteger(
            request->getParameter("patient_id"),
            patientId
        ))
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_query_parameter",
            "Patient ID must be a positive integer.",
            "patient_id"
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

    healthcare::services::AppointmentService
        appointmentService(databaseClient);

    appointmentService.listAppointments(
        status,
        doctorId,
        patientId,
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
                << "Failed to list appointments: "
                << errorMessage;

            sendError(
                sharedCallback,
                drogon::k500InternalServerError,
                "appointment_query_failed",
                "Unable to retrieve appointments."
            );
        }
    );
}

void AppointmentController::createAppointment(
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

    if (!requestBody->isMember("patient_id") ||
        !(*requestBody)["patient_id"].isInt())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_request_body",
            "Patient ID is required and must be an integer.",
            "patient_id"
        );

        return;
    }

    if (!requestBody->isMember("doctor_id") ||
        !(*requestBody)["doctor_id"].isInt())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_request_body",
            "Doctor ID is required and must be an integer.",
            "doctor_id"
        );

        return;
    }

    if (!requestBody->isMember(
            "appointment_datetime"
        ) ||
        !(*requestBody)["appointment_datetime"]
            .isString())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_request_body",
            "Appointment datetime is required and must be a string.",
            "appointment_datetime"
        );

        return;
    }

    healthcare::models::CreateAppointmentInput input;

    input.patientId =
        (*requestBody)["patient_id"].asInt();

    input.doctorId =
        (*requestBody)["doctor_id"].asInt();

    input.appointmentDatetime =
        (*requestBody)["appointment_datetime"]
            .asString();

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

    healthcare::services::AppointmentService
        appointmentService(databaseClient);

    appointmentService.createAppointment(
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
                AppointmentServiceError error
        )
        {
            switch (error.type)
            {
                case healthcare::services::
                    AppointmentServiceErrorType::
                        Validation:
                    sendError(
                        sharedCallback,
                        drogon::k400BadRequest,
                        "invalid_appointment_data",
                        error.message,
                        error.field
                    );
                    return;

                case healthcare::services::
                    AppointmentServiceErrorType::
                        NotFound:
                    sendError(
                        sharedCallback,
                        drogon::k404NotFound,
                        "appointment_reference_not_found",
                        error.message,
                        error.field
                    );
                    return;

                case healthcare::services::
                    AppointmentServiceErrorType::
                        Conflict:
                    sendError(
                        sharedCallback,
                        drogon::k409Conflict,
                        "appointment_slot_conflict",
                        error.message,
                        error.field
                    );
                    return;

                case healthcare::services::
                    AppointmentServiceErrorType::
                        Database:
                    LOG_ERROR
                        << "Failed to create appointment: "
                        << error.message;

                    sendError(
                        sharedCallback,
                        drogon::k500InternalServerError,
                        "appointment_creation_failed",
                        "Unable to create the appointment."
                    );
                    return;
            }
        }
    );
}

void AppointmentController::cancelAppointment(
    const drogon::HttpRequestPtr &,
    std::function<
        void(const drogon::HttpResponsePtr &)
    > &&callback,
    std::string appointmentId
) const
{
    auto sharedCallback =
        std::make_shared<ResponseCallback>(
            std::move(callback)
        );

    std::optional<int> parsedAppointmentId;

    if (!parseOptionalPositiveInteger(
            appointmentId,
            parsedAppointmentId
        ) ||
        !parsedAppointmentId.has_value())
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_path_parameter",
            "Appointment ID must be a positive integer.",
            "appointment_id"
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

    healthcare::services::AppointmentService
        appointmentService(databaseClient);

    appointmentService.cancelAppointment(
        parsedAppointmentId.value(),
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
            healthcare::services::
                AppointmentServiceError error
        )
        {
            switch (error.type)
            {
                case healthcare::services::
                    AppointmentServiceErrorType::
                        Validation:
                    sendError(
                        sharedCallback,
                        drogon::k400BadRequest,
                        "invalid_appointment_data",
                        error.message,
                        error.field
                    );
                    return;

                case healthcare::services::
                    AppointmentServiceErrorType::
                        NotFound:
                    sendError(
                        sharedCallback,
                        drogon::k404NotFound,
                        "appointment_not_found",
                        error.message,
                        error.field
                    );
                    return;

                case healthcare::services::
                    AppointmentServiceErrorType::
                        Conflict:
                    sendError(
                        sharedCallback,
                        drogon::k409Conflict,
                        "appointment_cancellation_conflict",
                        error.message,
                        error.field
                    );
                    return;

                case healthcare::services::
                    AppointmentServiceErrorType::
                        Database:
                    LOG_ERROR
                        << "Failed to cancel appointment: "
                        << error.message;

                    sendError(
                        sharedCallback,
                        drogon::k500InternalServerError,
                        "appointment_cancellation_failed",
                        "Unable to cancel the appointment."
                    );
                    return;
            }
        }
    );
}