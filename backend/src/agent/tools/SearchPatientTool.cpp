#include "agent/tools/SearchPatientTool.h"

#include "services/PatientService.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <utility>

namespace
{
std::string trim(
    const std::string &value
)
{
    const auto firstCharacter =
        std::find_if_not(
            value.begin(),
            value.end(),
            [](
                const unsigned char character
            )
            {
                return std::isspace(character);
            }
        );

    if (firstCharacter == value.end())
    {
        return "";
    }

    const auto lastCharacter =
        std::find_if_not(
            value.rbegin(),
            value.rend(),
            [](
                const unsigned char character
            )
            {
                return std::isspace(character);
            }
        ).base();

    return std::string(
        firstCharacter,
        lastCharacter
    );
}
}

namespace healthcare::agent::tools
{
SearchPatientTool::SearchPatientTool(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(
          std::move(databaseClient)
      )
{
}

std::string SearchPatientTool::name() const
{
    return "search_patient";
}

std::string SearchPatientTool::description() const
{
    return
        "Searches fictional clinic patients by full name "
        "or phone number.";
}

Json::Value SearchPatientTool::parametersSchema() const
{
    Json::Value queryProperty;

    queryProperty["type"] =
        "string";

    queryProperty["description"] =
        "A complete or partial patient name or phone number.";

    queryProperty["minLength"] =
        1;

    queryProperty["maxLength"] =
        120;

    Json::Value properties;
    properties["query"] =
        std::move(queryProperty);

    Json::Value required(
        Json::arrayValue
    );

    required.append("query");

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

void SearchPatientTool::execute(
    const Json::Value &arguments,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    if (!arguments.isObject())
    {
        errorCallback(
            {
                AgentToolErrorType::Validation,
                "arguments",
                "Tool arguments must be a JSON object."
            }
        );

        return;
    }

    if (!arguments.isMember("query") ||
        !arguments["query"].isString())
    {
        errorCallback(
            {
                AgentToolErrorType::Validation,
                "query",
                "Query is required and must be a string."
            }
        );

        return;
    }

    const std::string query =
        trim(
            arguments["query"].asString()
        );

    if (query.empty() ||
        query.size() > 120)
    {
        errorCallback(
            {
                AgentToolErrorType::Validation,
                "query",
                "Query must contain between 1 and 120 characters."
            }
        );

        return;
    }

    if (!databaseClient_)
    {
        errorCallback(
            {
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

    healthcare::services::PatientService
        patientService(databaseClient_);

    patientService.listPatients(
        query,
        [
            sharedSuccessCallback,
            query
        ](
            Json::Value patientResult
        )
        {
            Json::Value toolResult;

            toolResult["tool"] =
                "search_patient";

            toolResult["query"] =
                query;

            toolResult["result"] =
                std::move(patientResult);

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
                    AgentToolErrorType::Database,
                    "",
                    std::move(errorMessage)
                }
            );
        }
    );
}
}