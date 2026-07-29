#pragma once

#include "agent/AgentTool.h"

#include <json/json.h>

#include <map>
#include <memory>
#include <string>

namespace healthcare::agent
{
    class ToolRegistry final
    {
    public:
        void registerTool(
            std::shared_ptr<AgentTool> tool
        );

        [[nodiscard]]
        Json::Value definitions() const;

        [[nodiscard]]
        Json::Value geminiTools() const;

        void execute(
            const std::string &toolName,
            const Json::Value &arguments,
            AgentTool::SuccessCallback successCallback,
            AgentTool::ErrorCallback errorCallback
        ) const;

    private:
        std::map<
            std::string,
            std::shared_ptr<AgentTool>
        > tools_;
    };
}