#include "ai/GeminiResponse.h"

#include <json/json.h>

#include <iostream>
#include <optional>
#include <string>

namespace
{
int failedTests = 0;

void expect(
    const bool condition,
    const std::string &message
)
{
    if (condition)
    {
        return;
    }

    std::cerr
        << "FAILED: "
        << message
        << '\n';

    ++failedTests;
}

Json::Value createResponseWithParts(
    Json::Value parts
)
{
    Json::Value content;

    content["role"] =
        "model";

    content["parts"] =
        std::move(parts);

    Json::Value candidate;

    candidate["content"] =
        std::move(content);

    candidate["finishReason"] =
        "STOP";

    Json::Value candidates(
        Json::arrayValue
    );

    candidates.append(
        std::move(candidate)
    );

    Json::Value responseBody;

    responseBody["candidates"] =
        std::move(candidates);

    return responseBody;
}

Json::Value createFunctionCallPart(
    const std::string &id,
    const std::string &name,
    Json::Value arguments
)
{
    Json::Value functionCall;

    functionCall["id"] =
        id;

    functionCall["name"] =
        name;

    functionCall["args"] =
        std::move(arguments);

    Json::Value part;

    part["functionCall"] =
        std::move(functionCall);

    return part;
}

void testTextResponse()
{
    Json::Value textPart;

    textPart["text"] =
        "The backend is ready.";

    Json::Value parts(
        Json::arrayValue
    );

    parts.append(
        std::move(textPart)
    );

    const auto parsedResponse =
        healthcare::ai::
            GeminiResponseParser::parse(
                createResponseWithParts(
                    std::move(parts)
                )
            );

    expect(
        parsedResponse.has_value(),
        "A valid text response should be parsed."
    );

    if (!parsedResponse.has_value())
    {
        return;
    }

    expect(
        parsedResponse->type ==
            healthcare::ai::
                GeminiResponseType::Text,
        "A text response should have the Text type."
    );

    expect(
        parsedResponse->text ==
            "The backend is ready.",
        "The generated text should be preserved."
    );

    expect(
        parsedResponse->functionCalls.empty(),
        "A text response should contain no function calls."
    );
}

void testSingleFunctionCall()
{
    Json::Value arguments;

    arguments["query"] =
        "Sarah Cohen";

    Json::Value parts(
        Json::arrayValue
    );

    parts.append(
        createFunctionCallPart(
            "call-1",
            "search_patient",
            std::move(arguments)
        )
    );

    const auto parsedResponse =
        healthcare::ai::
            GeminiResponseParser::parse(
                createResponseWithParts(
                    std::move(parts)
                )
            );

    expect(
        parsedResponse.has_value(),
        "A valid function call should be parsed."
    );

    if (!parsedResponse.has_value())
    {
        return;
    }

    expect(
        parsedResponse->type ==
            healthcare::ai::
                GeminiResponseType::FunctionCall,
        "A function call should have the FunctionCall type."
    );

    expect(
        parsedResponse->functionCalls.size() == 1,
        "A single function call should produce one parsed call."
    );

    expect(
        parsedResponse->functionCall.id ==
            "call-1",
        "The compatibility function-call ID should be preserved."
    );

    expect(
        parsedResponse->functionCall.name ==
            "search_patient",
        "The compatibility function-call name should be preserved."
    );

    expect(
        parsedResponse->functionCall
                .arguments["query"]
                .asString() ==
            "Sarah Cohen",
        "The function-call arguments should be preserved."
    );
}

void testMultipleFunctionCalls()
{
    Json::Value patientArguments;

    patientArguments["query"] =
        "Sarah Cohen";

    Json::Value doctorArguments(
        Json::objectValue
    );

    Json::Value parts(
        Json::arrayValue
    );

    parts.append(
        createFunctionCallPart(
            "call-1",
            "search_patient",
            std::move(patientArguments)
        )
    );

    parts.append(
        createFunctionCallPart(
            "call-2",
            "list_doctors",
            std::move(doctorArguments)
        )
    );

    const auto parsedResponse =
        healthcare::ai::
            GeminiResponseParser::parse(
                createResponseWithParts(
                    std::move(parts)
                )
            );

    expect(
        parsedResponse.has_value(),
        "Multiple function calls should be parsed."
    );

    if (!parsedResponse.has_value())
    {
        return;
    }

    expect(
        parsedResponse->functionCalls.size() == 2,
        "Both Gemini function calls should be preserved."
    );

    if (parsedResponse->functionCalls.size() != 2)
    {
        return;
    }

    expect(
        parsedResponse->functionCalls[0].name ==
            "search_patient",
        "The first function call should preserve its order."
    );

    expect(
        parsedResponse->functionCalls[1].name ==
            "list_doctors",
        "The second function call should preserve its order."
    );

    expect(
        parsedResponse->functionCall.name ==
            "search_patient",
        "The compatibility field should contain the first call."
    );
}

void testModelContentPreservation()
{
    Json::Value arguments;

    arguments["query"] =
        "Sarah Cohen";

    Json::Value functionPart =
        createFunctionCallPart(
            "call-1",
            "search_patient",
            std::move(arguments)
        );

    functionPart["thoughtSignature"] =
        "signature-value";

    Json::Value parts(
        Json::arrayValue
    );

    parts.append(
        std::move(functionPart)
    );

    const auto parsedResponse =
        healthcare::ai::
            GeminiResponseParser::parse(
                createResponseWithParts(
                    std::move(parts)
                )
            );

    expect(
        parsedResponse.has_value(),
        "A function response with model metadata should parse."
    );

    if (!parsedResponse.has_value())
    {
        return;
    }

    expect(
        parsedResponse
                ->modelContent["parts"][0]
                ["thoughtSignature"]
                .asString() ==
            "signature-value",
        "The original model content and thought signature should be preserved."
    );
}

void testInvalidEmptyTextResponse()
{
    Json::Value textPart;

    textPart["text"] =
        "";

    Json::Value parts(
        Json::arrayValue
    );

    parts.append(
        std::move(textPart)
    );

    const std::optional<
        healthcare::ai::GeminiResponse
    > parsedResponse =
        healthcare::ai::
            GeminiResponseParser::parse(
                createResponseWithParts(
                    std::move(parts)
                )
            );

    expect(
        !parsedResponse.has_value(),
        "An empty text response should be rejected."
    );
}
}

int main()
{
    testTextResponse();
    testSingleFunctionCall();
    testMultipleFunctionCalls();
    testModelContentPreservation();
    testInvalidEmptyTextResponse();

    if (failedTests != 0)
    {
        std::cerr
            << failedTests
            << " Gemini response test(s) failed."
            << '\n';

        return 1;
    }

    std::cout
        << "All Gemini response tests passed."
        << '\n';

    return 0;
}