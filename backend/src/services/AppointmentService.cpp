#include "services/AppointmentService.h"

#include <utility>

namespace healthcare::services
{
AppointmentService::AppointmentService(
    drogon::orm::DbClientPtr databaseClient
)
    : appointmentRepository_(
          std::move(databaseClient)
      )
{
}

void AppointmentService::listAppointments(
    std::string status,
    std::optional<int> doctorId,
    std::optional<int> patientId,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    appointmentRepository_.findAll(
        std::move(status),
        doctorId,
        patientId,
        [
            successCallback =
                std::move(successCallback)
        ](
            healthcare::repositories::
                AppointmentRepository::AppointmentList
                    appointments
        ) mutable
        {
            Json::Value appointmentArray(
                Json::arrayValue
            );

            for (const auto &appointment : appointments)
            {
                Json::Value appointmentJson;

                appointmentJson["id"] =
                    appointment.id;

                appointmentJson["patient_id"] =
                    appointment.patientId;

                appointmentJson["patient_name"] =
                    appointment.patientName;

                appointmentJson["doctor_id"] =
                    appointment.doctorId;

                appointmentJson["doctor_name"] =
                    appointment.doctorName;

                appointmentJson["doctor_specialty"] =
                    appointment.doctorSpecialty;

                appointmentJson["appointment_datetime"] =
                    appointment.appointmentDatetime;

                appointmentJson["status"] =
                    appointment.status;

                appointmentJson["created_at"] =
                    appointment.createdAt;

                appointmentArray.append(
                    std::move(appointmentJson)
                );
            }

            Json::Value responseBody;

            responseBody["count"] =
                static_cast<Json::UInt64>(
                    appointments.size()
                );

            responseBody["data"] =
                std::move(appointmentArray);

            successCallback(
                std::move(responseBody)
            );
        },
        std::move(errorCallback)
    );
}
}