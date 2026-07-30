#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

#include <functional>

class GeminiController final
    : public drogon::HttpController<
          GeminiController
      >
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        GeminiController::checkConnection,
        "/health/gemini",
        drogon::Get
    );

    ADD_METHOD_TO(
        GeminiController::previewToolCall,
        "/agent/gemini/preview",
        drogon::Post
    );

    METHOD_LIST_END

    void checkConnection(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;

    void previewToolCall(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;
};