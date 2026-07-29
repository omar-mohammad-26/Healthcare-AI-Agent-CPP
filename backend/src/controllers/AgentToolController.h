#pragma once

#include <drogon/HttpController.h>

#include <functional>
#include <string>

class AgentToolController final
    : public drogon::HttpController<
          AgentToolController
      >
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        AgentToolController::listTools,
        "/agent/tools",
        drogon::Get
    );

    ADD_METHOD_TO(
        AgentToolController::executeTool,
        "/agent/tools/{1}",
        drogon::Post
    );

    METHOD_LIST_END

    void listTools(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;

    void executeTool(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback,
        std::string toolName
    ) const;
};