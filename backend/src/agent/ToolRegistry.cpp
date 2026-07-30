#include "agent/ToolRegistry.h"

#include <string>
#include <utility>

namespace
{
Json::Value createGeminiCompatibleSchema(
    const Json::Value &value
)
{
    if (value.isObject())
    {
        Json::Value sanitizedObject(
            Json::objectValue
        );

        for (const std::string &memberName :
             value.getMemberNames())
        {
            if (memberName == "additionalProperties")
            {
                continue;
            }

            sanitizedObject[memberName] =
                createGeminiCompatibleSchema(
                    value[memberName]
                );
        }

        return sanitizedObject;
    }

    if (value.isArray())
    {
        Json::Value sanitizedArray(
            Json::arrayValue
        );

        for (const auto &item : value)
        {
            sanitizedArray.append(
                createGeminiCompatibleSchema(
                    item
                )
            );
        }

        return sanitizedArray;
    }

    return value;
}

Json::Value createToolDefinition(
    const std::string &toolName,
    const healthcare::agent::AgentTool &tool
)
{
    Json::Value toolDefinition;

    toolDefinition["name"] =
        toolName;

    toolDefinition["description"] =
        tool.description();

    toolDefinition["parameters"] =
        tool.parametersSchema();

    return toolDefinition;
}

Json::Value createGeminiToolDefinition(
    const std::string &toolName,
    const healthcare::agent::AgentTool &tool
)
{
    Json::Value toolDefinition;

    toolDefinition["name"] =
        toolName;

    toolDefinition["description"] =
        tool.description();

    toolDefinition["parameters"] =
        createGeminiCompatibleSchema(
            tool.parametersSchema()
        );

    return toolDefinition;
}
}

namespace healthcare::agent
{
void ToolRegistry::registerTool(
    std::shared_ptr<AgentTool> tool
)
{
    if (!tool)
    {
        return;
    }

    const std::string toolName =
        tool->name();

    tools_[toolName] =
        std::move(tool);
}

Json::Value ToolRegistry::definitions() const
{
    Json::Value toolArray(
        Json::arrayValue
    );

    for (const auto &[toolName, tool] : tools_)
    {
        toolArray.append(
            createToolDefinition(
                toolName,
                *tool
            )
        );
    }

    Json::Value responseBody;

    responseBody["count"] =
        static_cast<Json::UInt64>(
            tools_.size()
        );

    responseBody["data"] =
        std::move(toolArray);

    return responseBody;
}

Json::Value ToolRegistry::geminiTools() const
{
    Json::Value geminiToolArray(
        Json::arrayValue
    );

    if (tools_.empty())
    {
        return geminiToolArray;
    }

    Json::Value functionDeclarations(
        Json::arrayValue
    );

    for (const auto &[toolName, tool] : tools_)
    {
        functionDeclarations.append(
            createGeminiToolDefinition(
                toolName,
                *tool
            )
        );
    }

    Json::Value functionTool;

    functionTool["functionDeclarations"] =
        std::move(functionDeclarations);

    geminiToolArray.append(
        std::move(functionTool)
    );

    return geminiToolArray;
}

void ToolRegistry::execute(
    const std::string &toolName,
    const Json::Value &arguments,
    AgentTool::SuccessCallback successCallback,
    AgentTool::ErrorCallback errorCallback
) const
{
    const auto toolIterator =
        tools_.find(toolName);

    if (toolIterator == tools_.end())
    {
        errorCallback(
            {
                AgentToolErrorType::NotFound,
                "tool_name",
                "The requested Agent tool is not registered."
            }
        );

        return;
    }

    toolIterator->second->execute(
        arguments,
        std::move(successCallback),
        std::move(errorCallback)
    );
}
}