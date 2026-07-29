#include "agent/tools/AvailableSlotsTool.h"

#include "services/DoctorService.h"

#include <cctype>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>

namespace
{
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
}

namespace healthcare::agent::tools
{
AvailableSlotsTool::AvailableSlotsTool(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(
          std::move(databaseClient)
      )
{
}

std::string AvailableSlotsTool::name() const
{
    return "available_slots";
}

std::string AvailableSlotsTool::description() const
{
    return
        "Returns the available 30-minute appointment "
        "slots for a doctor on a specific UTC date.";
}

Json::Value AvailableSlotsTool::parametersSchema() const
{
    Json::Value doctorIdProperty;

    doctorIdProperty["type"] =
        "integer";

    doctorIdProperty["description"] =
        "The unique ID of the doctor.";

    doctorIdProperty["minimum"] =
        1;

    Json::Value dateProperty;

    dateProperty["type"] =
        "string";

    dateProperty["description"] =
        "The requested UTC date in YYYY-MM-DD format.";

    dateProperty["pattern"] =
        "^\\d{4}-\\d{2}-\\d{2}$";

    Json::Value properties;

    properties["doctor_id"] =
        std::move(doctorIdProperty);

    properties["date"] =
        std::move(dateProperty);

    Json::Value required(
        Json::arrayValue
    );

    required.append("doctor_id");
    required.append("date");

    Json::Value schema;

    schema["type"] =
        "object";

    schema["properties"] =
        std::move(properties);

    schema["required"] =
        std::move(required);

    schema["additionalProperties"] =
        false;

    return schema;
}

void AvailableSlotsTool::execute(
    const Json::Value &arguments,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    if (!arguments.isObject())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "arguments",
                "Tool arguments must be a JSON object."
            }
        );

        return;
    }

    if (!arguments.isMember("doctor_id") ||
        !arguments["doctor_id"].isInt())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "doctor_id",
                "Doctor ID is required and must be an integer."
            }
        );

        return;
    }

    const int doctorId =
        arguments["doctor_id"].asInt();

    if (doctorId <= 0)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "doctor_id",
                "Doctor ID must be a positive integer."
            }
        );

        return;
    }

    if (!arguments.isMember("date") ||
        !arguments["date"].isString())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "date",
                "Date is required and must be a string."
            }
        );

        return;
    }

    const std::string date =
        arguments["date"].asString();

    if (!isValidDate(date))
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "date",
                "Date must use the YYYY-MM-DD format."
            }
        );

        return;
    }

    if (!databaseClient_)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Database,
                "",
                "The PostgreSQL client is not available."
            }
        );

        return;
    }

    auto sharedSuccessCallback =
        std::make_shared<SuccessCallback>(
            std::move(successCallback)
        );

    auto sharedErrorCallback =
        std::make_shared<ErrorCallback>(
            std::move(errorCallback)
        );

    healthcare::services::DoctorService
        doctorService(databaseClient_);

    doctorService.getAvailableSlots(
        doctorId,
        date,
        [
            sharedSuccessCallback,
            doctorId,
            date
        ](
            Json::Value availabilityResult
        )
        {
            Json::Value toolResult;

            toolResult["tool"] =
                "available_slots";

            toolResult["doctor_id"] =
                doctorId;

            toolResult["date"] =
                date;

            toolResult["result"] =
                std::move(availabilityResult);

            (*sharedSuccessCallback)(
                std::move(toolResult)
            );
        },
        [
            sharedErrorCallback
        ](
            healthcare::services::
                DoctorAvailabilityError error
        )
        {
            switch (error.type)
            {
                case healthcare::services::
                    DoctorAvailabilityErrorType::
                        NotFound:
                    (*sharedErrorCallback)(
                        {
                            healthcare::agent::
                                AgentToolErrorType::NotFound,
                            "doctor_id",
                            error.message
                        }
                    );
                    return;

                case healthcare::services::
                    DoctorAvailabilityErrorType::
                        Database:
                    (*sharedErrorCallback)(
                        {
                            healthcare::agent::
                                AgentToolErrorType::Database,
                            "",
                            error.message
                        }
                    );
                    return;
            }
        }
    );
}
}