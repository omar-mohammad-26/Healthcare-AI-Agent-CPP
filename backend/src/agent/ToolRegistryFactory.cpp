#include "agent/ToolRegistryFactory.h"

#include "agent/tools/AddTreatmentTool.h"
#include "agent/tools/AvailableSlotsTool.h"
#include "agent/tools/BookAppointmentTool.h"
#include "agent/tools/CancelAppointmentTool.h"
#include "agent/tools/DoctorScheduleTool.h"
#include "agent/tools/ListDoctorsTool.h"
#include "agent/tools/PatientHistoryTool.h"
#include "agent/tools/SearchPatientTool.h"

#include <memory>

namespace healthcare::agent
{
std::shared_ptr<ToolRegistry>
ToolRegistryFactory::create(
    const drogon::orm::DbClientPtr &databaseClient
)
{
    auto toolRegistry =
        std::make_shared<ToolRegistry>();

    toolRegistry->registerTool(
        std::make_shared<
            healthcare::agent::tools::SearchPatientTool
        >(
            databaseClient
        )
    );

    toolRegistry->registerTool(
        std::make_shared<
            healthcare::agent::tools::AvailableSlotsTool
        >(
            databaseClient
        )
    );

    toolRegistry->registerTool(
        std::make_shared<
            healthcare::agent::tools::BookAppointmentTool
        >(
            databaseClient
        )
    );

    toolRegistry->registerTool(
        std::make_shared<
            healthcare::agent::tools::CancelAppointmentTool
        >(
            databaseClient
        )
    );

    toolRegistry->registerTool(
        std::make_shared<
            healthcare::agent::tools::DoctorScheduleTool
        >(
            databaseClient
        )
    );

    toolRegistry->registerTool(
        std::make_shared<
            healthcare::agent::tools::PatientHistoryTool
        >(
            databaseClient
        )
    );

    toolRegistry->registerTool(
        std::make_shared<
            healthcare::agent::tools::AddTreatmentTool
        >(
            databaseClient
        )
    );

    toolRegistry->registerTool(
        std::make_shared<
            healthcare::agent::tools::ListDoctorsTool
        >(
            databaseClient
        )
    );

    return toolRegistry;
}
}