#pragma once

#include <drogon/HttpController.h>

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

    METHOD_LIST_END

    void checkConnection(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;
};