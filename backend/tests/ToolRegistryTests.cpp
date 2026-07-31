#include "agent/ToolRegistry.h"

#include <json/json.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>

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

class MockAgentTool final
    : public healthcare::agent::AgentTool
{
public:
    explicit MockAgentTool(
        std::string toolName
    )
        : toolName_(
              std::move(toolName)
          )
    {
    }

    [[nodiscard]]
    std::string name() const override
    {
        return toolName_;
    }

    [[nodiscard]]
    std::string description() const override
    {
        return
            "A mock Agent tool used for automated tests.";
    }

    [[nodiscard]]
    Json::Value parametersSchema() const override
    {
        Json::Value nestedProperties;

        nestedProperties["value"]["type"] =
            "string";

        Json::Value nestedObject;

        nestedObject["type"] =
            "object";

        nestedObject["properties"] =
            std::move(nestedProperties);

        nestedObject["additionalProperties"] =
            false;

        Json::Value properties;

        properties["query"]["type"] =
            "string";

        properties["options"] =
            std::move(nestedObject);

        Json::Value required(
            Json::arrayValue
        );

        required.append(
            "query"
        );

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

    void execute(
        const Json::Value &arguments,
        SuccessCallback successCallback,
        ErrorCallback errorCallback
    ) const override
    {
        static_cast<void>(
            errorCallback
        );

        Json::Value result;

        result["tool"] =
            toolName_;

        result["arguments"] =
            arguments;

        successCallback(
            std::move(result)
        );
    }

private:
    std::string toolName_;
};

void testNullToolIsIgnored()
{
    healthcare::agent::ToolRegistry registry;

    registry.registerTool(
        nullptr
    );

    const Json::Value definitions =
        registry.definitions();

    expect(
        definitions["count"].asUInt64() == 0,
        "Registering a null tool should not change the registry."
    );

    expect(
        definitions["data"].isArray() &&
            definitions["data"].empty(),
        "An empty registry should return an empty definitions array."
    );

    const Json::Value geminiTools =
        registry.geminiTools();

    expect(
        geminiTools.isArray() &&
            geminiTools.empty(),
        "An empty registry should return no Gemini tools."
    );
}

void testToolDefinitions()
{
    healthcare::agent::ToolRegistry registry;

    registry.registerTool(
        std::make_shared<MockAgentTool>(
            "mock_tool"
        )
    );

    const Json::Value definitions =
        registry.definitions();

    expect(
        definitions["count"].asUInt64() == 1,
        "The definitions response should report one registered tool."
    );

    expect(
        definitions["data"].isArray() &&
            definitions["data"].size() == 1,
        "The definitions response should contain one tool."
    );

    if (!definitions["data"].isArray() ||
        definitions["data"].size() != 1)
    {
        return;
    }

    const Json::Value &definition =
        definitions["data"][0];

    expect(
        definition["name"].asString() ==
            "mock_tool",
        "The tool definition should preserve the tool name."
    );

    expect(
        definition["description"].asString() ==
            "A mock Agent tool used for automated tests.",
        "The tool definition should preserve the description."
    );

    expect(
        definition["parameters"]
            .isMember("additionalProperties"),
        "Regular tool definitions should preserve additionalProperties."
    );

    expect(
        definition["parameters"]
            ["properties"]
            ["options"]
            .isMember("additionalProperties"),
        "Regular definitions should preserve nested additionalProperties."
    );
}

void testGeminiToolDefinitions()
{
    healthcare::agent::ToolRegistry registry;

    registry.registerTool(
        std::make_shared<MockAgentTool>(
            "mock_tool"
        )
    );

    const Json::Value geminiTools =
        registry.geminiTools();

    expect(
        geminiTools.isArray() &&
            geminiTools.size() == 1,
        "Gemini tools should contain one function-tool object."
    );

    if (!geminiTools.isArray() ||
        geminiTools.size() != 1)
    {
        return;
    }

    const Json::Value &declarations =
        geminiTools[0]
            ["functionDeclarations"];

    expect(
        declarations.isArray() &&
            declarations.size() == 1,
        "Gemini tools should contain one function declaration."
    );

    if (!declarations.isArray() ||
        declarations.size() != 1)
    {
        return;
    }

    const Json::Value &declaration =
        declarations[0];

    expect(
        declaration["name"].asString() ==
            "mock_tool",
        "The Gemini declaration should preserve the tool name."
    );

    expect(
        !declaration["parameters"]
            .isMember("additionalProperties"),
        "Gemini schemas should remove root additionalProperties."
    );

    expect(
        !declaration["parameters"]
            ["properties"]
            ["options"]
            .isMember("additionalProperties"),
        "Gemini schemas should remove nested additionalProperties."
    );

    expect(
        declaration["parameters"]
            ["properties"]
            ["query"]
            ["type"]
            .asString() ==
            "string",
        "Gemini schema sanitization should preserve valid properties."
    );
}

void testRegisteredToolExecution()
{
    healthcare::agent::ToolRegistry registry;

    registry.registerTool(
        std::make_shared<MockAgentTool>(
            "mock_tool"
        )
    );

    Json::Value arguments;

    arguments["query"] =
        "Sarah Cohen";

    bool successCalled = false;
    bool errorCalled = false;

    registry.execute(
        "mock_tool",
        arguments,
        [
            &successCalled
        ](
            Json::Value result
        )
        {
            successCalled = true;

            expect(
                result["tool"].asString() ==
                    "mock_tool",
                "The registered tool should execute successfully."
            );

            expect(
                result["arguments"]
                    ["query"]
                    .asString() ==
                    "Sarah Cohen",
                "The registry should forward tool arguments unchanged."
            );
        },
        [
            &errorCalled
        ](
            healthcare::agent::AgentToolError
        )
        {
            errorCalled = true;
        }
    );

    expect(
        successCalled,
        "Executing a registered tool should invoke the success callback."
    );

    expect(
        !errorCalled,
        "Executing a registered tool should not invoke the error callback."
    );
}

void testUnknownToolExecution()
{
    healthcare::agent::ToolRegistry registry;

    bool successCalled = false;
    bool errorCalled = false;

    registry.execute(
        "missing_tool",
        Json::Value(
            Json::objectValue
        ),
        [
            &successCalled
        ](
            Json::Value
        )
        {
            successCalled = true;
        },
        [
            &errorCalled
        ](
            healthcare::agent::AgentToolError error
        )
        {
            errorCalled = true;

            expect(
                error.type ==
                    healthcare::agent::
                        AgentToolErrorType::NotFound,
                "An unknown tool should return a NotFound error."
            );

            expect(
                error.field ==
                    "tool_name",
                "An unknown tool error should identify tool_name."
            );

            expect(
                error.message ==
                    "The requested Agent tool is not registered.",
                "An unknown tool should return the expected message."
            );
        }
    );

    expect(
        !successCalled,
        "An unknown tool should not invoke the success callback."
    );

    expect(
        errorCalled,
        "An unknown tool should invoke the error callback."
    );
}

void testDuplicateToolReplacement()
{
    healthcare::agent::ToolRegistry registry;

    registry.registerTool(
        std::make_shared<MockAgentTool>(
            "duplicate_tool"
        )
    );

    registry.registerTool(
        std::make_shared<MockAgentTool>(
            "duplicate_tool"
        )
    );

    const Json::Value definitions =
        registry.definitions();

    expect(
        definitions["count"].asUInt64() == 1,
        "Registering the same tool name twice should replace the existing tool."
    );
}
}

int main()
{
    testNullToolIsIgnored();
    testToolDefinitions();
    testGeminiToolDefinitions();
    testRegisteredToolExecution();
    testUnknownToolExecution();
    testDuplicateToolReplacement();

    if (failedTests != 0)
    {
        std::cerr
            << failedTests
            << " ToolRegistry test(s) failed."
            << '\n';

        return 1;
    }

    std::cout
        << "All ToolRegistry tests passed."
        << '\n';

    return 0;
}