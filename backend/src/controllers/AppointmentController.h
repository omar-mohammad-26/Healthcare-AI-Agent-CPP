#pragma once

#include <drogon/HttpController.h>

#include <functional>

class AppointmentController final
    : public drogon::HttpController<
          AppointmentController
      >
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        AppointmentController::listAppointments,
        "/appointments",
        drogon::Get
    );

    METHOD_LIST_END

    void listAppointments(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;
};