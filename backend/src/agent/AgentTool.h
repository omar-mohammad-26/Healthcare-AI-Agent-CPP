#pragma once

#include <json/json.h>

#include <functional>
#include <string>

namespace healthcare::agent
{
    enum class AgentToolErrorType
    {
        Validation,
        NotFound,
        Conflict,
        Database
    };

    struct AgentToolError
    {
        AgentToolErrorType type;
        std::string field;
        std::string message;
    };

    class AgentTool
    {
    public:
        using SuccessCallback =
            std::function<void(Json::Value)>;

        using ErrorCallback =
            std::function<void(AgentToolError)>;

        virtual ~AgentTool() = default;

        [[nodiscard]]
        virtual std::string name() const = 0;

        [[nodiscard]]
        virtual std::string description() const = 0;

        [[nodiscard]]
        virtual Json::Value parametersSchema() const = 0;

        virtual void execute(
            const Json::Value &arguments,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const = 0;
    };
}