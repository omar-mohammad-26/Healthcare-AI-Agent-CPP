#pragma once

#include "agent/ToolRegistry.h"

#include <drogon/orm/DbClient.h>

#include <memory>

namespace healthcare::agent
{
    class ToolRegistryFactory final
    {
    public:
        [[nodiscard]]
        static std::shared_ptr<ToolRegistry> create(
            const drogon::orm::DbClientPtr &databaseClient
        );
    };
}