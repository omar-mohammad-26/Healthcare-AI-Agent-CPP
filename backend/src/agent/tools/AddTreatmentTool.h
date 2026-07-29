#pragma once

#include "agent/AgentTool.h"

#include <drogon/orm/DbClient.h>

#include <string>

namespace healthcare::agent::tools
{
    class AddTreatmentTool final
        : public healthcare::agent::AgentTool
    {
    public:
        explicit AddTreatmentTool(
            drogon::orm::DbClientPtr databaseClient
        );

        [[nodiscard]]
        std::string name() const override;

        [[nodiscard]]
        std::string description() const override;

        [[nodiscard]]
        Json::Value parametersSchema() const override;

        void execute(
            const Json::Value &arguments,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const override;

    private:
        drogon::orm::DbClientPtr databaseClient_;
    };
}