#pragma once

#include <drogon/HttpController.h>

#include <functional>

class TreatmentController final
    : public drogon::HttpController<
          TreatmentController
      >
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        TreatmentController::listTreatments,
        "/treatments",
        drogon::Get
    );

    METHOD_LIST_END

    void listTreatments(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;
};