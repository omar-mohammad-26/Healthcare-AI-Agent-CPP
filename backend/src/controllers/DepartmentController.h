#pragma once

#include <drogon/HttpController.h>

#include <functional>

class DepartmentController final
    : public drogon::HttpController<
          DepartmentController
      >
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        DepartmentController::listDepartments,
        "/departments",
        drogon::Get
    );

    METHOD_LIST_END

    void listDepartments(
        const drogon::HttpRequestPtr &request,
        std::function<
            void(const drogon::HttpResponsePtr &)
        > &&callback
    ) const;
};