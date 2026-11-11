#include "SimModeGo2.h"

#include "AirBlueprintLib.h"
#include "common/AirSimSettings.hpp"
#include "common/EarthUtils.hpp"

#include "Go2PawnSimApi.h"
#include "Go2RpcLibServer.h"

extern CORE_API uint32 GFrameNumber;

void ASimModeGo2::BeginPlay()
{
    Super::BeginPlay();

    initializePauseState();
}

void ASimModeGo2::initializePauseState()
{
    pause_period_ = 0;
    pause_period_start_ = 0;
    pause(false);
}

void ASimModeGo2::continueForTime(double seconds)
{
    pause_period_start_ = ClockFactory::get()->nowNanos();
    pause_period_ = seconds * current_clockspeed_;
    pause(false);
}

void ASimModeGo2::continueForFrames(uint32_t frames)
{
    targetFrameNumber_ = GFrameNumber + frames;
    frame_countdown_enabled_ = true;
    pause(false);
}

void ASimModeGo2::setupClockSpeed()
{
    Super::setupClockSpeed();

    current_clockspeed_ = getSettings().clock_speed;

    UAirBlueprintLib::setUnrealClockSpeed(this, current_clockspeed_);
    UAirBlueprintLib::LogMessageString("Clock Speed: ", std::to_string(current_clockspeed_), LogDebugLevel::Informational);
}

void ASimModeGo2::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!isPaused())
        ClockFactory::get()->stepBy(DeltaSeconds);

    // Rendering-only mode: drive the pawn with the joint targets that were
    // received over RPC (applied inside Go2PawnSimApi::updateRendering).
    for (auto& api : getApiProvider()->getVehicleSimApis()) {
        api->updateRenderedState(DeltaSeconds);
        api->updateRendering(DeltaSeconds);
    }

    if (pause_period_start_ > 0) {
        if (ClockFactory::get()->elapsedSince(pause_period_start_) >= pause_period_) {
            if (!isPaused())
                pause(true);

            pause_period_start_ = 0;
        }
    }

    if (frame_countdown_enabled_) {
        if (targetFrameNumber_ <= GFrameNumber) {
            if (!isPaused())
                pause(true);

            frame_countdown_enabled_ = false;
        }
    }
}

std::unique_ptr<msr::airlib::ApiServerBase> ASimModeGo2::createApiServer() const
{
#ifdef AIRLIB_NO_RPC
    return ASimModeBase::createApiServer();
#else
    return std::unique_ptr<msr::airlib::ApiServerBase>(new msr::airlib::Go2RpcLibServer(
        getApiProvider(), getSettings().api_server_address, getSettings().api_port));
#endif
}

void ASimModeGo2::getExistingVehiclePawns(TArray<AActor*>& pawns) const
{
    UAirBlueprintLib::FindAllActor<TVehiclePawn>(this, pawns);
}

bool ASimModeGo2::isVehicleTypeSupported(const std::string& vehicle_type) const
{
    return vehicle_type == msr::airlib::AirSimSettings::kVehicleTypeGo2;
}

std::string ASimModeGo2::getVehiclePawnPathName(const AirSimSettings::VehicleSetting& vehicle_setting) const
{
    std::string pawn_path = vehicle_setting.pawn_path;
    if (pawn_path == "")
        pawn_path = "DefaultGo2";

    return pawn_path;
}

PawnEvents* ASimModeGo2::getVehiclePawnEvents(APawn* pawn) const
{
    return static_cast<TVehiclePawn*>(pawn)->getPawnEvents();
}

const common_utils::UniqueValueMap<std::string, APIPCamera*> ASimModeGo2::getVehiclePawnCameras(APawn* pawn) const
{
    return static_cast<const TVehiclePawn*>(pawn)->getCameras();
}

void ASimModeGo2::initializeVehiclePawn(APawn* pawn)
{
    static_cast<TVehiclePawn*>(pawn)->initializeForBeginPlay();
}

std::unique_ptr<PawnSimApi> ASimModeGo2::createVehicleSimApi(
    const PawnSimApi::Params& pawn_sim_api_params) const
{
    auto vehicle_sim_api = std::unique_ptr<PawnSimApi>(new Go2PawnSimApi(pawn_sim_api_params));
    vehicle_sim_api->initialize();
    vehicle_sim_api->reset();
    return vehicle_sim_api;
}

msr::airlib::VehicleApiBase* ASimModeGo2::getVehicleApi(const PawnSimApi::Params& pawn_sim_api_params,
                                                       const PawnSimApi* sim_api) const
{
    unused(pawn_sim_api_params);

    return static_cast<const Go2PawnSimApi*>(sim_api)->getVehicleApiBase();
}
