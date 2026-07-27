#include "controllers/AppointmentController.h"

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
                std::numeric_limits<int>::max())
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

void sendBadRequest(
    const SharedResponseCallback &callback,
    const std::string &parameter,
    const std::string &message
)
{
    Json::Value responseBody;
    responseBody["status"] = "error";
    responseBody["code"] =
        "invalid_query_parameter";
    responseBody["parameter"] =
        parameter;
    responseBody["message"] =
        message;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(
            responseBody
        );

    response->setStatusCode(
        drogon::k400BadRequest
    );

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
        sendBadRequest(
            sharedCallback,
            "status",
            "Status must be scheduled, cancelled, or completed."
        );

        return;
    }

    std::optional<int> doctorId;

    if (!parseOptionalPositiveInteger(
            request->getParameter("doctor_id"),
            doctorId
        ))
    {
        sendBadRequest(
            sharedCallback,
            "doctor_id",
            "Doctor ID must be a positive integer."
        );

        return;
    }

    std::optional<int> patientId;

    if (!parseOptionalPositiveInteger(
            request->getParameter("patient_id"),
            patientId
        ))
    {
        sendBadRequest(
            sharedCallback,
            "patient_id",
            "Patient ID must be a positive integer."
        );

        return;
    }

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

            Json::Value responseBody;
            responseBody["status"] = "error";
            responseBody["code"] =
                "appointment_query_failed";
            responseBody["message"] =
                "Unable to retrieve appointments.";

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