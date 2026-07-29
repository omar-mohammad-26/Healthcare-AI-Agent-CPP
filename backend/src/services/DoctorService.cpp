#include "services/DoctorService.h"

#include <memory>
#include <string>
#include <utility>

namespace healthcare::services
{
DoctorService::DoctorService(
    drogon::orm::DbClientPtr databaseClient
)
    : doctorRepository_(
          std::move(databaseClient)
      )
{
}

void DoctorService::listDoctors(
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    doctorRepository_.findAll(
        [
            successCallback =
                std::move(successCallback)
        ](
            healthcare::repositories::
                DoctorRepository::DoctorList doctors
        ) mutable
        {
            Json::Value doctorArray(
                Json::arrayValue
            );

            for (const auto &doctor : doctors)
            {
                Json::Value doctorJson;

                doctorJson["id"] =
                    doctor.id;

                doctorJson["full_name"] =
                    doctor.fullName;

                doctorJson["specialty"] =
                    doctor.specialty;

                doctorJson["department_id"] =
                    doctor.departmentId;

                doctorJson["department_name"] =
                    doctor.departmentName;

                doctorArray.append(
                    std::move(doctorJson)
                );
            }

            Json::Value responseBody;

            responseBody["count"] =
                static_cast<Json::UInt64>(
                    doctors.size()
                );

            responseBody["data"] =
                std::move(doctorArray);

            successCallback(
                std::move(responseBody)
            );
        },
        std::move(errorCallback)
    );
}

void DoctorService::getDoctorSchedule(
    const int doctorId,
    std::string date,
    SuccessCallback successCallback,
    ScheduleErrorCallback errorCallback
) const
{
    auto sharedSuccessCallback =
        std::make_shared<SuccessCallback>(
            std::move(successCallback)
        );

    auto sharedErrorCallback =
        std::make_shared<ScheduleErrorCallback>(
            std::move(errorCallback)
        );

    doctorRepository_.findSchedule(
        doctorId,
        std::move(date),
        [
            sharedSuccessCallback,
            sharedErrorCallback
        ](
            healthcare::models::DoctorScheduleResult
                result
        )
        {
            if (result.outcome ==
                healthcare::models::
                    DoctorScheduleOutcome::
                        DoctorNotFound)
            {
                (*sharedErrorCallback)(
                    {
                        DoctorScheduleErrorType::NotFound,
                        "The requested doctor does not exist."
                    }
                );

                return;
            }

            const auto &schedule =
                result.schedule.value();

            Json::Value doctorJson;

            doctorJson["id"] =
                schedule.doctorId;

            doctorJson["full_name"] =
                schedule.doctorName;

            doctorJson["specialty"] =
                schedule.specialty;

            doctorJson["department_name"] =
                schedule.departmentName;

            Json::Value appointmentArray(
                Json::arrayValue
            );

            for (const auto &appointment :
                 schedule.appointments)
            {
                Json::Value appointmentJson;

                appointmentJson["id"] =
                    appointment.appointmentId;

                appointmentJson["patient_id"] =
                    appointment.patientId;

                appointmentJson["patient_name"] =
                    appointment.patientName;

                appointmentJson["appointment_datetime"] =
                    appointment.appointmentDatetime;

                appointmentJson["status"] =
                    appointment.status;

                appointmentArray.append(
                    std::move(appointmentJson)
                );
            }

            Json::Value responseBody;

            responseBody["doctor"] =
                std::move(doctorJson);

            responseBody["date"] =
                schedule.date;

            responseBody["timezone"] =
                "UTC";

            responseBody["count"] =
                static_cast<Json::UInt64>(
                    schedule.appointments.size()
                );

            responseBody["data"] =
                std::move(appointmentArray);

            (*sharedSuccessCallback)(
                std::move(responseBody)
            );
        },
        [
            sharedErrorCallback
        ](
            std::string errorMessage
        )
        {
            (*sharedErrorCallback)(
                {
                    DoctorScheduleErrorType::Database,
                    std::move(errorMessage)
                }
            );
        }
    );
}
}