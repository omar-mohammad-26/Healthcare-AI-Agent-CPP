#include "ai/GeminiResponse.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <optional>
#include <string>
#include <utility>

namespace
{
std::string jsonToString(
    const Json::Value &value
)
{
    Json::StreamWriterBuilder writerBuilder;

    writerBuilder["indentation"] =
        "  ";

    return Json::writeString(
        writerBuilder,
        value
    );
}

std::optional<healthcare::ai::GeminiResponse>
logInvalidResponse(
    const std::string &reason,
    const Json::Value &responseBody
)
{
    LOG_ERROR
        << "Unable to parse Gemini response: "
        << reason
        << "\nGemini response body:\n"
        << jsonToString(responseBody);

    return std::nullopt;
}
}

namespace healthcare::ai
{
std::optional<GeminiResponse>
GeminiResponseParser::parse(
    const Json::Value &responseBody
)
{
    if (!responseBody.isObject())
    {
        return logInvalidResponse(
            "The response body is not a JSON object.",
            responseBody
        );
    }

    if (!responseBody.isMember("candidates") ||
        !responseBody["candidates"].isArray() ||
        responseBody["candidates"].empty())
    {
        return logInvalidResponse(
            "The response contains no candidates.",
            responseBody
        );
    }

    const Json::Value &candidate =
        responseBody["candidates"][0];

    if (!candidate.isObject())
    {
        return logInvalidResponse(
            "The first candidate is not a JSON object.",
            responseBody
        );
    }

    if (!candidate.isMember("content") ||
        !candidate["content"].isObject())
    {
        return logInvalidResponse(
            "The first candidate contains no model content.",
            responseBody
        );
    }

    const Json::Value &content =
        candidate["content"];

    if (!content.isMember("parts") ||
        !content["parts"].isArray() ||
        content["parts"].empty())
    {
        return logInvalidResponse(
            "The model content contains no parts.",
            responseBody
        );
    }

    const Json::Value &parts =
        content["parts"];

    GeminiResponse parsedResponse;

    parsedResponse.modelContent =
        content;

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

        GeminiFunctionCall parsedFunctionCall;

        parsedFunctionCall.name =
            functionName;

        if (functionCall.isMember("id") &&
            functionCall["id"].isString())
        {
            parsedFunctionCall.id =
                functionCall["id"].asString();
        }

        if (functionCall.isMember("args") &&
            functionCall["args"].isObject())
        {
            parsedFunctionCall.arguments =
                functionCall["args"];
        }

        parsedResponse.functionCalls.push_back(
            std::move(parsedFunctionCall)
        );
    }

    if (!parsedResponse.functionCalls.empty())
    {
        parsedResponse.type =
            GeminiResponseType::FunctionCall;

        parsedResponse.functionCall =
            parsedResponse.functionCalls.front();

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
        return logInvalidResponse(
            "The response contains neither generated text nor a valid function call.",
            responseBody
        );
    }

    parsedResponse.type =
        GeminiResponseType::Text;

    parsedResponse.text =
        std::move(generatedText);

    return parsedResponse;
}
}