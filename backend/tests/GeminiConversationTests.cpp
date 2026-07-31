#include "ai/GeminiConversation.h"

#include <json/json.h>

#include <iostream>
#include <string>
#include <vector>

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

void testConversationCreation()
{
    const Json::Value contents =
        healthcare::ai::
            GeminiConversation::create(
                "Find Sarah Cohen."
            );

    expect(
        contents.isArray(),
        "A Gemini conversation should be a JSON array."
    );

    expect(
        contents.size() == 1,
        "A new conversation should contain one user message."
    );

    if (!contents.isArray() ||
        contents.size() != 1)
    {
        return;
    }

    const Json::Value &userContent =
        contents[0];

    expect(
        userContent["role"].asString() ==
            "user",
        "The first conversation message should have the user role."
    );

    expect(
        userContent["parts"].isArray() &&
            userContent["parts"].size() == 1,
        "The user message should contain one text part."
    );

    expect(
        userContent["parts"][0]["text"]
                .asString() ==
            "Find Sarah Cohen.",
        "The original prompt should be preserved."
    );
}

void testModelContentAppend()
{
    Json::Value contents =
        healthcare::ai::
            GeminiConversation::create(
                "List all doctors."
            );

    Json::Value functionCall;

    functionCall["id"] =
        "call-1";

    functionCall["name"] =
        "list_doctors";

    functionCall["args"] =
        Json::Value(
            Json::objectValue
        );

    Json::Value part;

    part["functionCall"] =
        functionCall;

    part["thoughtSignature"] =
        "signature-value";

    Json::Value parts(
        Json::arrayValue
    );

    parts.append(
        part
    );

    Json::Value modelContent;

    modelContent["role"] =
        "model";

    modelContent["parts"] =
        parts;

    healthcare::ai::
        GeminiConversation::appendModelContent(
            contents,
            modelContent
        );

    expect(
        contents.size() == 2,
        "Appending model content should add one conversation message."
    );

    if (contents.size() != 2)
    {
        return;
    }

    expect(
        contents[1]["role"].asString() ==
            "model",
        "Appended model content should preserve the model role."
    );

    expect(
        contents[1]["parts"][0]
                ["functionCall"]
                ["name"]
                .asString() ==
            "list_doctors",
        "Appended model content should preserve the function call."
    );

    expect(
        contents[1]["parts"][0]
                ["thoughtSignature"]
                .asString() ==
            "signature-value",
        "Appended model content should preserve thought signatures."
    );
}

void testSingleFunctionResponseAppend()
{
    Json::Value contents =
        healthcare::ai::
            GeminiConversation::create(
                "Find Sarah Cohen."
            );

    Json::Value toolResult;

    toolResult["tool"] =
        "search_patient";

    toolResult["count"] =
        1;

    healthcare::ai::
        GeminiConversation::
            appendFunctionResponse(
                contents,
                "search_patient",
                toolResult,
                "call-1"
            );

    expect(
        contents.size() == 2,
        "Appending one function response should add one user message."
    );

    if (contents.size() != 2)
    {
        return;
    }

    const Json::Value &responseContent =
        contents[1];

    expect(
        responseContent["role"].asString() ==
            "user",
        "Function responses should be sent using the user role."
    );

    expect(
        responseContent["parts"].isArray() &&
            responseContent["parts"].size() == 1,
        "A single function response should produce one part."
    );

    const Json::Value &functionResponse =
        responseContent["parts"][0]
            ["functionResponse"];

    expect(
        functionResponse["id"].asString() ==
            "call-1",
        "The function-call ID should be preserved."
    );

    expect(
        functionResponse["name"].asString() ==
            "search_patient",
        "The function name should be preserved."
    );

    expect(
        functionResponse["response"]
                ["count"]
                .asInt() ==
            1,
        "An object tool result should be preserved directly."
    );
}

void testMultipleFunctionResponsesAppend()
{
    Json::Value contents =
        healthcare::ai::
            GeminiConversation::create(
                "Find Sarah Cohen and list doctors."
            );

    Json::Value patientResult;

    patientResult["count"] =
        1;

    Json::Value doctorResult;

    doctorResult["count"] =
        8;

    std::vector<
        healthcare::ai::GeminiFunctionResult
    > functionResults;

    healthcare::ai::GeminiFunctionResult
        patientFunctionResult;

    patientFunctionResult.functionCallId =
        "call-1";

    patientFunctionResult.functionName =
        "search_patient";

    patientFunctionResult.functionResult =
        patientResult;

    functionResults.push_back(
        patientFunctionResult
    );

    healthcare::ai::GeminiFunctionResult
        doctorFunctionResult;

    doctorFunctionResult.functionCallId =
        "call-2";

    doctorFunctionResult.functionName =
        "list_doctors";

    doctorFunctionResult.functionResult =
        doctorResult;

    functionResults.push_back(
        doctorFunctionResult
    );

    healthcare::ai::
        GeminiConversation::
            appendFunctionResponses(
                contents,
                functionResults
            );

    expect(
        contents.size() == 2,
        "Multiple function results should be appended in one user message."
    );

    if (contents.size() != 2)
    {
        return;
    }

    const Json::Value &parts =
        contents[1]["parts"];

    expect(
        parts.isArray() &&
            parts.size() == 2,
        "Two function results should produce two response parts."
    );

    if (!parts.isArray() ||
        parts.size() != 2)
    {
        return;
    }

    expect(
        parts[0]["functionResponse"]
                ["name"]
                .asString() ==
            "search_patient",
        "The first function response should preserve its order."
    );

    expect(
        parts[1]["functionResponse"]
                ["name"]
                .asString() ==
            "list_doctors",
        "The second function response should preserve its order."
    );

    expect(
        parts[0]["functionResponse"]
                ["id"]
                .asString() ==
            "call-1",
        "The first function response should preserve its ID."
    );

    expect(
        parts[1]["functionResponse"]
                ["id"]
                .asString() ==
            "call-2",
        "The second function response should preserve its ID."
    );
}

void testNonObjectFunctionResult()
{
    Json::Value contents =
        healthcare::ai::
            GeminiConversation::create(
                "Run a simple tool."
            );

    healthcare::ai::
        GeminiConversation::
            appendFunctionResponse(
                contents,
                "simple_tool",
                Json::Value("completed"),
                "call-3"
            );

    expect(
        contents.size() == 2,
        "A scalar function result should still append a response."
    );

    if (contents.size() != 2)
    {
        return;
    }

    expect(
        contents[1]["parts"][0]
                ["functionResponse"]
                ["response"]
                ["result"]
                .asString() ==
            "completed",
        "A non-object tool result should be wrapped in a result field."
    );
}

void testInvalidInputsAreIgnored()
{
    Json::Value invalidContents(
        Json::objectValue
    );

    Json::Value modelContent;

    modelContent["role"] =
        "model";

    healthcare::ai::
        GeminiConversation::appendModelContent(
            invalidContents,
            modelContent
        );

    expect(
        invalidContents.empty(),
        "Model content should not be appended to a non-array conversation."
    );

    Json::Value validContents =
        healthcare::ai::
            GeminiConversation::create(
                "Test invalid responses."
            );

    const Json::ArrayIndex originalSize =
        validContents.size();

    healthcare::ai::
        GeminiConversation::
            appendFunctionResponse(
                validContents,
                "",
                Json::Value(
                    Json::objectValue
                )
            );

    expect(
        validContents.size() ==
            originalSize,
        "A function response with an empty name should be ignored."
    );

    const std::vector<
        healthcare::ai::GeminiFunctionResult
    > emptyResults;

    healthcare::ai::
        GeminiConversation::
            appendFunctionResponses(
                validContents,
                emptyResults
            );

    expect(
        validContents.size() ==
            originalSize,
        "An empty function-result collection should be ignored."
    );
}
}

int main()
{
    testConversationCreation();
    testModelContentAppend();
    testSingleFunctionResponseAppend();
    testMultipleFunctionResponsesAppend();
    testNonObjectFunctionResult();
    testInvalidInputsAreIgnored();

    if (failedTests != 0)
    {
        std::cerr
            << failedTests
            << " Gemini conversation test(s) failed."
            << '\n';

        return 1;
    }

    std::cout
        << "All Gemini conversation tests passed."
        << '\n';

    return 0;
}