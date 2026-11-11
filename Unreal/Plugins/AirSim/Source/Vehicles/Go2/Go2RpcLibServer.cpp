#include "Go2RpcLibServer.h"
#include "Go2PawnSimApi.h"

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
STRICT_MODE_ON

namespace msr
{
namespace airlib
{
    Go2RpcLibServer::Go2RpcLibServer(ApiProvider* api_provider, const std::string& server_address, uint16_t port)
        : RpcLibServerBase(api_provider, server_address, port)
    {
        auto* server = static_cast<rpc::server*>(getServer());

        server->bind("setGo2JointAngles", [this](const std::vector<float>& angles, const std::string& vehicle_name) -> void {
            getGo2SimApi(vehicle_name)->setJointAngles(angles);
        });
        server->bind("getGo2JointAngles", [this](const std::string& vehicle_name) -> std::vector<float> {
            return getGo2SimApi(vehicle_name)->getJointAngles();
        });
        server->bind("setGo2BaseOffset", [this](const std::vector<float>& offset, const std::string& vehicle_name) -> void {
            getGo2SimApi(vehicle_name)->setBaseOffset(offset);
        });
    }

    Go2RpcLibServer::~Go2RpcLibServer()
    {
    }

    ::Go2PawnSimApi* Go2RpcLibServer::getGo2SimApi(const std::string& vehicle_name)
    {
        auto* api = getVehicleSimApi(vehicle_name);
        auto* go2_api = static_cast<::Go2PawnSimApi*>(api);
        if (go2_api == nullptr)
            throw ApiNotSupported("Go2 sim API is not available for vehicle '" + vehicle_name + "'");
        return go2_api;
    }
}
}
