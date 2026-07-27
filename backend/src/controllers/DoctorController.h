#pragma once

#include <drogon/HttpController.h>

#include <functional>

class DoctorController final
    : public drogon::HttpController<
          DoctorController
      >
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        DoctorController::listDoctors,
        "/doctors",
        drogon::Get
    );

    METHOD_LIST_END

    void listDoctors(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;
};