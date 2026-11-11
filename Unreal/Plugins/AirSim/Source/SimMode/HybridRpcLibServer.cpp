#include "HybridRpcLibServer.h"

#include "common/Common.hpp"
STRICT_MODE_OFF

#ifndef RPCLIB_MSGPACK
#define RPCLIB_MSGPACK clmdep_msgpack
#endif
#include "common/common_utils/MinWinDefines.hpp"
#undef NOUSER

#include "common/common_utils/WindowsApisCommonPre.hpp"
#undef FLOAT
#undef check
#include "rpc/server.h"
#ifndef check
#define check(expr) (static_cast<void>((expr)))
#endif
#include "common/common_utils/WindowsApisCommonPost.hpp"

#include "vehicles/car/api/CarRpcLibAdaptors.hpp"
STRICT_MODE_ON

#include "common/AirSimSettings.hpp"

namespace msr
{
namespace airlib
{
    typedef msr::airlib_rpclib::CarRpcLibAdaptors CarRpcLibAdaptors;

    static bool isCarVehicleType(const std::string& vehicle_type)
    {
        return vehicle_type == AirSimSettings::kVehicleTypePhysXCar ||
               vehicle_type == AirSimSettings::kVehicleTypeBoxCar ||
               vehicle_type == AirSimSettings::kVehicleTypeArduRover;
    }

    HybridRpcLibServer::HybridRpcLibServer(ApiProvider* api_provider, const std::string& server_address, uint16_t port)
        : MultirotorRpcLibServer(api_provider, server_address, port)
        , provider_(api_provider)
    {
        auto* server = static_cast<rpc::server*>(getServer());

        server->bind("getCarState", [this](const std::string& vehicle_name) -> CarRpcLibAdaptors::CarState {
            return CarRpcLibAdaptors::CarState(getCarApi(vehicle_name)->getCarState());
        });
        server->bind("setCarControls", [this](const CarRpcLibAdaptors::CarControls& controls, const std::string& vehicle_name) -> void {
            getCarApi(vehicle_name)->setCarControls(controls.to());
        });
        server->bind("getCarControls", [this](const std::string& vehicle_name) -> CarRpcLibAdaptors::CarControls {
            return CarRpcLibAdaptors::CarControls(getCarApi(vehicle_name)->getCarControls());
        });
    }

    HybridRpcLibServer::~HybridRpcLibServer()
    {
    }

    std::string HybridRpcLibServer::resolveVehicleName(const std::string& vehicle_name) const
    {
        // An empty name refers to the default vehicle; resolve it to the real
        // name so we can look up its type in the settings.
        if (!vehicle_name.empty())
            return vehicle_name;

        auto* sim_api = provider_->getVehicleSimApi("");
        return sim_api != nullptr ? sim_api->getVehicleName() : vehicle_name;
    }

    MultirotorApiBase* HybridRpcLibServer::getVehicleApi(const std::string& vehicle_name)
    {
        // This is used both by multirotor-specific bindings and by the generic
        // bindings in RpcLibServerBase (enableApiControl, setPose, ...). The
        // generic ones only call VehicleApiBase methods, so returning the
        // vehicle API pointer is sufficient there; for cars the virtual calls
        // still dispatch to the car implementation. Multirotor-specific RPCs
        // are only valid when the vehicle really is a multirotor.
        auto* api = provider_->getVehicleApi(vehicle_name);
        if (api == nullptr)
            throw ApiNotSupported("Vehicle API is not available for vehicle '" + vehicle_name + "'");

        return static_cast<MultirotorApiBase*>(api);
    }

    CarApiBase* HybridRpcLibServer::getCarApi(const std::string& vehicle_name)
    {
        const std::string resolved_name = resolveVehicleName(vehicle_name);
        const auto& vehicles = AirSimSettings::singleton().vehicles;
        auto it = vehicles.find(resolved_name);
        if (it == vehicles.end() || !isCarVehicleType(it->second->vehicle_type))
            throw ApiNotSupported("Car API is not available for vehicle '" + vehicle_name + "'");

        auto* api = provider_->getVehicleApi(vehicle_name);
        if (api == nullptr)
            throw ApiNotSupported("Car API is not available for vehicle '" + vehicle_name + "'");

        return static_cast<CarApiBase*>(api);
    }
}
}
