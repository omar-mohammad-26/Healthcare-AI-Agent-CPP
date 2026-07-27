#pragma once

#include "repositories/DoctorRepository.h"

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include <functional>
#include <string>

namespace healthcare::services
{
    class DoctorService final
    {
    public:
        using SuccessCallback =
            std::function<void(Json::Value)>;

        using ErrorCallback =
            std::function<void(std::string)>;

        explicit DoctorService(
            drogon::orm::DbClientPtr databaseClient
        );

        void listDoctors(
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        healthcare::repositories::DoctorRepository
            doctorRepository_;
    };
}