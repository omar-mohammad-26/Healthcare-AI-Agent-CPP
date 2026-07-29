#include "agent/tools/BookAppointmentTool.h"

#include "models/Appointment.h"
#include "services/AppointmentService.h"

#include <memory>
#include <string>
#include <utility>

namespace healthcare::agent::tools
{
BookAppointmentTool::BookAppointmentTool(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(
          std::move(databaseClient)
      )
{
}

std::string BookAppointmentTool::name() const
{
    return "book_appointment";
}

std::string BookAppointmentTool::description() const
{
    return
        "Books a future appointment for a fictional clinic "
        "patient with a selected doctor and UTC datetime.";
}

Json::Value BookAppointmentTool::parametersSchema() const
{
    Json::Value patientIdProperty;

    patientIdProperty["type"] =
        "integer";

    patientIdProperty["description"] =
        "The unique ID of the patient.";

    patientIdProperty["minimum"] =
        1;

    Json::Value doctorIdProperty;

    doctorIdProperty["type"] =
        "integer";

    doctorIdProperty["description"] =
        "The unique ID of the doctor.";

    doctorIdProperty["minimum"] =
        1;

    Json::Value appointmentDatetimeProperty;

    appointmentDatetimeProperty["type"] =
        "string";

    appointmentDatetimeProperty["description"] =
        "The future appointment datetime in ISO 8601 format.";

    appointmentDatetimeProperty["pattern"] =
        "^\\d{4}-\\d{2}-\\d{2}T"
        "\\d{2}:\\d{2}:\\d{2}"
        "(Z|[+-]\\d{2}:\\d{2})$";

    Json::Value properties;

    properties["patient_id"] =
        std::move(patientIdProperty);

    properties["doctor_id"] =
        std::move(doctorIdProperty);

    properties["appointment_datetime"] =
        std::move(appointmentDatetimeProperty);

    Json::Value required(
        Json::arrayValue
    );

    required.append("patient_id");
    required.append("doctor_id");
    required.append("appointment_datetime");

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

void BookAppointmentTool::execute(
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

    if (!arguments.isMember("patient_id") ||
        !arguments["patient_id"].isInt())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "patient_id",
                "Patient ID is required and must be an integer."
            }
        );

        return;
    }

    const int patientId =
        arguments["patient_id"].asInt();

    if (patientId <= 0)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "patient_id",
                "Patient ID must be a positive integer."
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

    if (!arguments.isMember(
            "appointment_datetime"
        ) ||
        !arguments["appointment_datetime"]
            .isString())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "appointment_datetime",
                "Appointment datetime is required and must be a string."
            }
        );

        return;
    }

    const std::string appointmentDatetime =
        arguments["appointment_datetime"]
            .asString();

    if (appointmentDatetime.empty() ||
        appointmentDatetime.size() > 40)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "appointment_datetime",
                "Appointment datetime must contain a valid ISO 8601 value."
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

    healthcare::models::CreateAppointmentInput input;

    input.patientId =
        patientId;

    input.doctorId =
        doctorId;

    input.appointmentDatetime =
        appointmentDatetime;

    auto sharedSuccessCallback =
        std::make_shared<SuccessCallback>(
            std::move(successCallback)
        );

    auto sharedErrorCallback =
        std::make_shared<ErrorCallback>(
            std::move(errorCallback)
        );

    healthcare::services::AppointmentService
        appointmentService(databaseClient_);

    appointmentService.createAppointment(
        std::move(input),
        [
            sharedSuccessCallback,
            patientId,
            doctorId,
            appointmentDatetime
        ](
            Json::Value appointmentResult
        )
        {
            Json::Value toolResult;

            toolResult["tool"] =
                "book_appointment";

            toolResult["patient_id"] =
                patientId;

            toolResult["doctor_id"] =
                doctorId;

            toolResult["appointment_datetime"] =
                appointmentDatetime;

            toolResult["result"] =
                std::move(appointmentResult);

            (*sharedSuccessCallback)(
                std::move(toolResult)
            );
        },
        [
            sharedErrorCallback
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
                    (*sharedErrorCallback)(
                        {
                            healthcare::agent::
                                AgentToolErrorType::
                                    Validation,
                            error.field,
                            error.message
                        }
                    );
                    return;

                case healthcare::services::
                    AppointmentServiceErrorType::
                        NotFound:
                    (*sharedErrorCallback)(
                        {
                            healthcare::agent::
                                AgentToolErrorType::
                                    NotFound,
                            error.field,
                            error.message
                        }
                    );
                    return;

                case healthcare::services::
                    AppointmentServiceErrorType::
                        Conflict:
                    (*sharedErrorCallback)(
                        {
                            healthcare::agent::
                                AgentToolErrorType::
                                    Conflict,
                            error.field,
                            error.message
                        }
                    );
                    return;

                case healthcare::services::
                    AppointmentServiceErrorType::
                        Database:
                    (*sharedErrorCallback)(
                        {
                            healthcare::agent::
                                AgentToolErrorType::
                                    Database,
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