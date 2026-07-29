#include "agent/tools/ListDoctorsTool.h"

#include "services/DoctorService.h"

#include <memory>
#include <string>
#include <utility>

namespace healthcare::agent::tools
{
ListDoctorsTool::ListDoctorsTool(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(
          std::move(databaseClient)
      )
{
}

std::string ListDoctorsTool::name() const
{
    return "list_doctors";
}

std::string ListDoctorsTool::description() const
{
    return
        "Returns all fictional clinic doctors with "
        "their specialties and departments.";
}

Json::Value ListDoctorsTool::parametersSchema() const
{
    Json::Value schema;

    schema["type"] =
        "object";

    schema["properties"] =
        Json::Value(Json::objectValue);

    schema["additionalProperties"] =
        false;

    return schema;
}

void ListDoctorsTool::execute(
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

    if (!arguments.getMemberNames().empty())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "arguments",
                "The list_doctors tool does not accept arguments."
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

    doctorService.listDoctors(
        [
            sharedSuccessCallback
        ](
            Json::Value doctorResult
        )
        {
            Json::Value toolResult;

            toolResult["tool"] =
                "list_doctors";

            toolResult["result"] =
                std::move(doctorResult);

            (*sharedSuccessCallback)(
                std::move(toolResult)
            );
        },
        [
            sharedErrorCallback
        ](
            std::string errorMessage
        )
        {
            (*sharedErrorCallback)(
                {
                    healthcare::agent::
                        AgentToolErrorType::Database,
                    "",
                    std::move(errorMessage)
                }
            );
        }
    );
}
}