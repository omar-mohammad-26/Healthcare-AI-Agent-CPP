#pragma once

#include <drogon/HttpController.h>

#include <functional>
#include <string>

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

    ADD_METHOD_TO(
        DoctorController::getDoctorSchedule,
        "/doctors/{1}/schedule",
        drogon::Get
    );

    METHOD_LIST_END

    void listDoctors(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;

    void getDoctorSchedule(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback,
        std::string doctorId
    ) const;
};