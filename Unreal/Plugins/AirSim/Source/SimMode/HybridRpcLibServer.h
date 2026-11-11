#pragma once

#include "vehicles/multirotor/api/MultirotorRpcLibServer.hpp"
#include "vehicles/car/api/CarApiBase.hpp"

namespace msr
{
namespace airlib
{
    // Multirotor RPC server extended with the car-specific RPC methods so that
    // MultirotorClient and CarClient can both talk to the same port.
    class HybridRpcLibServer : public MultirotorRpcLibServer
    {
    public:
        HybridRpcLibServer(ApiProvider* api_provider, const std::string& server_address, uint16_t port = RpcLibPort);
        virtual ~HybridRpcLibServer() override;

    protected:
        virtual MultirotorApiBase* getVehicleApi(const std::string& vehicle_name) override;

    private:
        CarApiBase* getCarApi(const std::string& vehicle_name);
        std::string resolveVehicleName(const std::string& vehicle_name) const;

        ApiProvider* provider_;
    };
}
}
