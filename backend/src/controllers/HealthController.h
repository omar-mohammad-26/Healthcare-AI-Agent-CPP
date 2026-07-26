#pragma once

#include <drogon/HttpSimpleController.h>

#include <functional>

class HealthController final
    : public drogon::HttpSimpleController<HealthController>
{
public:
    void asyncHandleHttpRequest(
        const drogon::HttpRequestPtr &request,
        std::function<void(const drogon::HttpResponsePtr &)> &&callback
    ) override;

    PATH_LIST_BEGIN
    PATH_ADD("/health", drogon::Get);
    PATH_LIST_END
};