#pragma once

#include <drogon/HttpSimpleController.h>

#include <functional>

class DatabaseHealthController final
    : public drogon::HttpSimpleController<
          DatabaseHealthController
      >
{
public:
    void asyncHandleHttpRequest(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) override;

    PATH_LIST_BEGIN
    PATH_ADD("/health/database", drogon::Get);
    PATH_LIST_END
};