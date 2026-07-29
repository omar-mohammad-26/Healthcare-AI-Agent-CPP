#pragma once

#include <drogon/HttpController.h>

#include <functional>
#include <string>

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

    ADD_METHOD_TO(
        AppointmentController::createAppointment,
        "/appointments",
        drogon::Post
    );

    ADD_METHOD_TO(
        AppointmentController::cancelAppointment,
        "/appointments/{1}/cancel",
        drogon::Post
    );

    METHOD_LIST_END

    void listAppointments(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;

    void createAppointment(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;

    void cancelAppointment(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback,
        std::string appointmentId
    ) const;
};