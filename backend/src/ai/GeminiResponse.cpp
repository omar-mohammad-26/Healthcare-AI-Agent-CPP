#include "ai/GeminiResponse.h"

#include <optional>
#include <string>
#include <utility>

namespace healthcare::ai
{
std::optional<GeminiResponse>
GeminiResponseParser::parse(
    const Json::Value &responseBody
)
{
    if (!responseBody.isObject() ||
        !responseBody.isMember("candidates") ||
        !responseBody["candidates"].isArray() ||
        responseBody["candidates"].empty())
    {
        return std::nullopt;
    }

    const Json::Value &candidate =
        responseBody["candidates"][0];

    if (!candidate.isObject() ||
        !candidate.isMember("content") ||
        !candidate["content"].isObject())
    {
        return std::nullopt;
    }

    const Json::Value &content =
        candidate["content"];

    if (!content.isMember("parts") ||
        !content["parts"].isArray() ||
        content["parts"].empty())
    {
        return std::nullopt;
    }

    const Json::Value &parts =
        content["parts"];

    for (const auto &part : parts)
    {
        if (!part.isObject() ||
            !part.isMember("functionCall") ||
            !part["functionCall"].isObject())
        {
            continue;
        }

        const Json::Value &functionCall =
            part["functionCall"];

        if (!functionCall.isMember("name") ||
            !functionCall["name"].isString())
        {
            continue;
        }

        const std::string functionName =
            functionCall["name"].asString();

        if (functionName.empty())
        {
            continue;
        }

        GeminiResponse parsedResponse;

        parsedResponse.type =
            GeminiResponseType::FunctionCall;

        parsedResponse.modelContent =
            content;

        parsedResponse.functionCall.name =
            functionName;

        if (functionCall.isMember("id") &&
            functionCall["id"].isString())
        {
            parsedResponse.functionCall.id =
                functionCall["id"].asString();
        }

        if (functionCall.isMember("args") &&
            functionCall["args"].isObject())
        {
            parsedResponse.functionCall.arguments =
                functionCall["args"];
        }

        return parsedResponse;
    }

    std::string generatedText;

    for (const auto &part : parts)
    {
        if (!part.isObject() ||
            !part.isMember("text") ||
            !part["text"].isString())
        {
            continue;
        }

        generatedText +=
            part["text"].asString();
    }

    if (generatedText.empty())
    {
        return std::nullopt;
    }

    GeminiResponse parsedResponse;

    parsedResponse.type =
        GeminiResponseType::Text;

    parsedResponse.text =
        std::move(generatedText);

    parsedResponse.modelContent =
        content;

    return parsedResponse;
}
}