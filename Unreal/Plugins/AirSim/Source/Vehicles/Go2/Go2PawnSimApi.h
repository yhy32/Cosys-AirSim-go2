#pragma once

#include <array>
#include <atomic>
#include <vector>

#include "CoreMinimal.h"

#include "PawnSimApi.h"
#include "Go2Pawn.h"
#include "Go2Api.h"

// Sim API for the Go2 pawn. Joint angles arrive from an external simulator
// (MuJoCo) through the Go2 RPC server and are applied on the render tick.
class Go2PawnSimApi : public PawnSimApi
{
public:
    Go2PawnSimApi(const Params& params);

    virtual void initialize() override;
    virtual void updateRendering(float dt) override;
    virtual std::string getRecordFileLine(bool is_header_line) const override;

    msr::airlib::Go2Api* getVehicleApi() const
    {
        return vehicle_api_.get();
    }

    virtual msr::airlib::VehicleApiBase* getVehicleApiBase() const override
    {
        return vehicle_api_.get();
    }

    void setJointAngles(const std::vector<float>& angles);
    std::vector<float> getJointAngles() const;

    void setBaseOffset(const std::vector<float>& offset);

private:
    AGo2Pawn* go2_pawn_ = nullptr;
    std::unique_ptr<msr::airlib::Go2Api> vehicle_api_;
    std::array<std::atomic<float>, 12> target_angles_;
    std::array<float, 12> applied_angles_;
    std::array<std::atomic<float>, 3> base_offset_;
    std::array<float, 3> applied_base_offset_;
};
