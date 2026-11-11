#pragma once

#include "api/RpcLibServerBase.hpp"

class Go2PawnSimApi;

namespace msr
{
namespace airlib
{
    // RPC server for the Go2 mode. In addition to the common AirSim bindings
    // it exposes setGo2JointAngles/getGo2JointAngles so an external simulator
    // (MuJoCo) can drive the 12 joints.
    class Go2RpcLibServer : public RpcLibServerBase
    {
    public:
        Go2RpcLibServer(ApiProvider* api_provider, const std::string& server_address, uint16_t port = RpcLibPort);
        virtual ~Go2RpcLibServer() override;

    protected:
        ::Go2PawnSimApi* getGo2SimApi(const std::string& vehicle_name);
    };
}
}
