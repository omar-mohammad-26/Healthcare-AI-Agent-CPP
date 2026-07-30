#include "ai/GeminiConversation.h"

#include <string>
#include <utility>

namespace healthcare::ai
{
Json::Value GeminiConversation::create(
    const std::string &prompt
)
{
    Json::Value textPart;

    textPart["text"] =
        prompt;

    Json::Value parts(
        Json::arrayValue
    );

    parts.append(
        std::move(textPart)
    );

    Json::Value userContent;

    userContent["role"] =
        "user";

    userContent["parts"] =
        std::move(parts);

    Json::Value contents(
        Json::arrayValue
    );

    contents.append(
        std::move(userContent)
    );

    return contents;
}

void GeminiConversation::appendModelContent(
    Json::Value &contents,
    const Json::Value &modelContent
)
{
    if (!contents.isArray() ||
        !modelContent.isObject())
    {
        return;
    }

    contents.append(
        modelContent
    );
}

void GeminiConversation::appendFunctionResponse(
    Json::Value &contents,
    const std::string &functionName,
    const Json::Value &functionResult,
    const std::string &functionCallId
)
{
    if (!contents.isArray() ||
        functionName.empty())
    {
        return;
    }

    Json::Value responseValue;

    if (functionResult.isObject())
    {
        responseValue =
            functionResult;
    }
    else
    {
        responseValue["result"] =
            functionResult;
    }

    Json::Value functionResponse;

    functionResponse["name"] =
        functionName;

    functionResponse["response"] =
        std::move(responseValue);

    if (!functionCallId.empty())
    {
        functionResponse["id"] =
            functionCallId;
    }

    Json::Value responsePart;

    responsePart["functionResponse"] =
        std::move(functionResponse);

    Json::Value parts(
        Json::arrayValue
    );

    parts.append(
        std::move(responsePart)
    );

    Json::Value userContent;

    userContent["role"] =
        "user";

    userContent["parts"] =
        std::move(parts);

    contents.append(
        std::move(userContent)
    );
}
}