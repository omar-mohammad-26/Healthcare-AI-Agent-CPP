#include "controllers/DoctorController.h"

#include "services/DoctorService.h"

#include <drogon/drogon.h>
#include <json/json.h>

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

bool parsePositiveInteger(
    const std::string &value,
    int &parsedValue
)
{
    if (value.empty())
    {
        return false;
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

bool isDigitAt(
    const std::string &value,
    const std::size_t position
)
{
    return position < value.size() &&
           std::isdigit(
               static_cast<unsigned char>(
                   value[position]
               )
           );
}

int parseTwoDigits(
    const std::string &value,
    const std::size_t position
)
{
    return
        (value[position] - '0') * 10 +
        (value[position + 1] - '0');
}

int parseFourDigits(
    const std::string &value,
    const std::size_t position
)
{
    return
        (value[position] - '0') * 1000 +
        (value[position + 1] - '0') * 100 +
        (value[position + 2] - '0') * 10 +
        (value[position + 3] - '0');
}

bool isLeapYear(
    const int year
)
{
    return
        year % 400 == 0 ||
        (year % 4 == 0 && year % 100 != 0);
}

int daysInMonth(
    const int year,
    const int month
)
{
    switch (month)
    {
        case 2:
            return isLeapYear(year) ? 29 : 28;

        case 4:
        case 6:
        case 9:
        case 11:
            return 30;

        default:
            return 31;
    }
}

bool isValidDate(
    const std::string &value
)
{
    if (value.size() != 10)
    {
        return false;
    }

    const std::size_t digitPositions[] =
    {
        0, 1, 2, 3,
        5, 6,
        8, 9
    };

    for (const auto position : digitPositions)
    {
        if (!isDigitAt(value, position))
        {
            return false;
        }
    }

    if (value[4] != '-' ||
        value[7] != '-')
    {
        return false;
    }

    const int year =
        parseFourDigits(value, 0);

    const int month =
        parseTwoDigits(value, 5);

    const int day =
        parseTwoDigits(value, 8);

    return
        year >= 2000 &&
        year <= 2200 &&
        month >= 1 &&
        month <= 12 &&
        day >= 1 &&
        day <= daysInMonth(year, month);
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

void DoctorController::listDoctors(
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

            sendError(
                sharedCallback,
                drogon::k500InternalServerError,
                "doctor_query_failed",
                "Unable to retrieve doctors."
            );
        }
    );
}

void DoctorController::getDoctorSchedule(
    const drogon::HttpRequestPtr &request,
    std::function<
        void(const drogon::HttpResponsePtr &)
    > &&callback,
    std::string doctorId
) const
{
    auto sharedCallback =
        std::make_shared<ResponseCallback>(
            std::move(callback)
        );

    int parsedDoctorId = 0;

    if (!parsePositiveInteger(
            doctorId,
            parsedDoctorId
        ))
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_path_parameter",
            "Doctor ID must be a positive integer.",
            "doctor_id"
        );

        return;
    }

    const std::string date =
        request->getParameter("date");

    if (!isValidDate(date))
    {
        sendError(
            sharedCallback,
            drogon::k400BadRequest,
            "invalid_query_parameter",
            "Date is required and must use YYYY-MM-DD format.",
            "date"
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

    healthcare::services::DoctorService
        doctorService(databaseClient);

    doctorService.getDoctorSchedule(
        parsedDoctorId,
        date,
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
                DoctorScheduleError error
        )
        {
            switch (error.type)
            {
                case healthcare::services::
                    DoctorScheduleErrorType::NotFound:
                    sendError(
                        sharedCallback,
                        drogon::k404NotFound,
                        "doctor_not_found",
                        error.message,
                        "doctor_id"
                    );
                    return;

                case healthcare::services::
                    DoctorScheduleErrorType::Database:
                    LOG_ERROR
                        << "Failed to retrieve doctor schedule: "
                        << error.message;

                    sendError(
                        sharedCallback,
                        drogon::k500InternalServerError,
                        "doctor_schedule_query_failed",
                        "Unable to retrieve the doctor schedule."
                    );
                    return;
            }
        }
    );
}