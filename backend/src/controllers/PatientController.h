#pragma once

#include <drogon/HttpController.h>

#include <functional>

class PatientController final
    : public drogon::HttpController<
          PatientController
      >
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        PatientController::listPatients,
        "/patients",
        drogon::Get
    );

    ADD_METHOD_TO(
        PatientController::createPatient,
        "/patients",
        drogon::Post
    );

    METHOD_LIST_END

    void listPatients(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;

    void createPatient(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;
};