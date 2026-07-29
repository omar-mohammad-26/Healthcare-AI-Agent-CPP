find_package(Drogon CONFIG REQUIRED)

set(BACKEND_SOURCES
        src/main.cpp

        src/config/DatabaseConfig.cpp
        src/config/DatabaseConfig.h

        src/config/GeminiConfig.cpp
        src/config/GeminiConfig.h

        src/ai/GeminiClient.cpp
        src/ai/GeminiClient.h

        src/models/Department.h
        src/models/Doctor.h
        src/models/Patient.h
        src/models/Appointment.h
        src/models/Treatment.h

        src/repositories/DepartmentRepository.cpp
        src/repositories/DepartmentRepository.h
        src/repositories/DoctorRepository.cpp
        src/repositories/DoctorRepository.h
        src/repositories/PatientRepository.cpp
        src/repositories/PatientRepository.h
        src/repositories/AppointmentRepository.cpp
        src/repositories/AppointmentRepository.h
        src/repositories/TreatmentRepository.cpp
        src/repositories/TreatmentRepository.h

        src/services/DepartmentService.cpp
        src/services/DepartmentService.h
        src/services/DoctorService.cpp
        src/services/DoctorService.h
        src/services/PatientService.cpp
        src/services/PatientService.h
        src/services/AppointmentService.cpp
        src/services/AppointmentService.h
        src/services/TreatmentService.cpp
        src/services/TreatmentService.h

        src/agent/AgentTool.h
        src/agent/ToolRegistry.cpp
        src/agent/ToolRegistry.h

        src/agent/tools/SearchPatientTool.cpp
        src/agent/tools/SearchPatientTool.h

        src/agent/tools/AvailableSlotsTool.cpp
        src/agent/tools/AvailableSlotsTool.h

        src/agent/tools/BookAppointmentTool.cpp
        src/agent/tools/BookAppointmentTool.h

        src/agent/tools/CancelAppointmentTool.cpp
        src/agent/tools/CancelAppointmentTool.h

        src/agent/tools/DoctorScheduleTool.cpp
        src/agent/tools/DoctorScheduleTool.h

        src/agent/tools/PatientHistoryTool.cpp
        src/agent/tools/PatientHistoryTool.h

        src/agent/tools/AddTreatmentTool.cpp
        src/agent/tools/AddTreatmentTool.h

        src/agent/tools/ListDoctorsTool.cpp
        src/agent/tools/ListDoctorsTool.h

        src/controllers/HealthController.cpp
        src/controllers/HealthController.h

        src/controllers/DatabaseHealthController.cpp
        src/controllers/DatabaseHealthController.h

        src/controllers/GeminiController.cpp
        src/controllers/GeminiController.h

        src/controllers/DepartmentController.cpp
        src/controllers/DepartmentController.h

        src/controllers/DoctorController.cpp
        src/controllers/DoctorController.h

        src/controllers/PatientController.cpp
        src/controllers/PatientController.h

        src/controllers/AppointmentController.cpp
        src/controllers/AppointmentController.h

        src/controllers/TreatmentController.cpp
        src/controllers/TreatmentController.h

        src/controllers/AgentToolController.cpp
        src/controllers/AgentToolController.h
)

add_executable(
        healthcare_ai_agent
        ${BACKEND_SOURCES}
)

target_include_directories(
        healthcare_ai_agent
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
        healthcare_ai_agent
        PRIVATE
        Drogon::Drogon
)

target_compile_features(
        healthcare_ai_agent
        PRIVATE
        cxx_std_20
)

if(MSVC)
    target_compile_options(
            healthcare_ai_agent
            PRIVATE
            /W4
            /permissive-
    )
else()
    target_compile_options(
            healthcare_ai_agent
            PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/config.json
        ${CMAKE_CURRENT_BINARY_DIR}/config.json
        COPYONLY
)