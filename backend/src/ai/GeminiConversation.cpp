#include "ai/GeminiConversation.h"

#include <string>
#include <utility>
#include <vector>

namespace
{
Json::Value createFunctionResponsePart(
    const healthcare::ai::GeminiFunctionResult
        &functionResult
)
{
    Json::Value responseValue;

    if (functionResult.functionResult.isObject())
    {
        responseValue =
            functionResult.functionResult;
    }
    else
    {
        responseValue["result"] =
            functionResult.functionResult;
    }

    Json::Value functionResponse;

    functionResponse["name"] =
        functionResult.functionName;

    functionResponse["response"] =
        std::move(responseValue);

    if (!functionResult.functionCallId.empty())
    {
        functionResponse["id"] =
            functionResult.functionCallId;
    }

    Json::Value responsePart;

    responsePart["functionResponse"] =
        std::move(functionResponse);

    return responsePart;
}
}

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
    GeminiFunctionResult result;

    result.functionCallId =
        functionCallId;

    result.functionName =
        functionName;

    result.functionResult =
        functionResult;

    appendFunctionResponses(
        contents,
        {result}
    );
}

void GeminiConversation::appendFunctionResponses(
    Json::Value &contents,
    const std::vector<GeminiFunctionResult>
        &functionResults
)
{
    if (!contents.isArray() ||
        functionResults.empty())
    {
        return;
    }

    Json::Value parts(
        Json::arrayValue
    );

    for (const auto &functionResult :
         functionResults)
    {
        if (functionResult.functionName.empty())
        {
            continue;
        }

        parts.append(
            createFunctionResponsePart(
                functionResult
            )
        );
    }

    if (parts.empty())
    {
        return;
    }

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