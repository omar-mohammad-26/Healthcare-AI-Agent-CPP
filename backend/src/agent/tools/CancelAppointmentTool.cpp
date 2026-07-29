#include "agent/tools/CancelAppointmentTool.h"

#include "services/AppointmentService.h"

#include <memory>
#include <string>
#include <utility>

namespace healthcare::agent::tools
{
CancelAppointmentTool::CancelAppointmentTool(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(
          std::move(databaseClient)
      )
{
}

std::string CancelAppointmentTool::name() const
{
    return "cancel_appointment";
}

std::string CancelAppointmentTool::description() const
{
    return
        "Cancels an existing scheduled appointment "
        "using its unique appointment ID.";
}

Json::Value CancelAppointmentTool::parametersSchema() const
{
    Json::Value appointmentIdProperty;

    appointmentIdProperty["type"] =
        "integer";

    appointmentIdProperty["description"] =
        "The unique ID of the appointment to cancel.";

    appointmentIdProperty["minimum"] =
        1;

    Json::Value properties;

    properties["appointment_id"] =
        std::move(appointmentIdProperty);

    Json::Value required(
        Json::arrayValue
    );

    required.append("appointment_id");

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

void CancelAppointmentTool::execute(
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

    if (!arguments.isMember("appointment_id") ||
        !arguments["appointment_id"].isInt())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "appointment_id",
                "Appointment ID is required and must be an integer."
            }
        );

        return;
    }

    const int appointmentId =
        arguments["appointment_id"].asInt();

    if (appointmentId <= 0)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "appointment_id",
                "Appointment ID must be a positive integer."
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

    healthcare::services::AppointmentService
        appointmentService(databaseClient_);

    appointmentService.cancelAppointment(
        appointmentId,
        [
            sharedSuccessCallback,
            appointmentId
        ](
            Json::Value cancellationResult
        )
        {
            Json::Value toolResult;

            toolResult["tool"] =
                "cancel_appointment";

            toolResult["appointment_id"] =
                appointmentId;

            toolResult["result"] =
                std::move(cancellationResult);

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