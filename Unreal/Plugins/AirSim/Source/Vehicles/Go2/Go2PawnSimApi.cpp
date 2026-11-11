#include "Go2PawnSimApi.h"

#include "AirBlueprintLib.h"

Go2PawnSimApi::Go2PawnSimApi(const Params& params)
    : PawnSimApi(params)
    , go2_pawn_(Cast<AGo2Pawn>(params.pawn))
{
    for (int i = 0; i < 12; ++i) {
        const float value = (i % 3 == 1) ? 0.9f : (i % 3 == 2) ? -1.8f : 0.0f;
        target_angles_[i].store(value);
        applied_angles_[i] = value;
    }
    for (int i = 0; i < 3; ++i) {
        base_offset_[i].store(0.0f);
        applied_base_offset_[i] = 0.0f;
    }
}

void Go2PawnSimApi::initialize()
{
    PawnSimApi::initialize();

    const auto home_geopoint = getGroundTruthEnvironment()->getHomeGeoPoint();
    vehicle_api_.reset(new msr::airlib::Go2Api(getVehicleSetting(), home_geopoint));
}

void Go2PawnSimApi::updateRendering(float dt)
{
    PawnSimApi::updateRendering(dt);

    if (go2_pawn_ == nullptr)
        return;

    bool changed = false;
    TArray<float> angles;
    angles.SetNumUninitialized(12);
    for (int i = 0; i < 12; ++i) {
        angles[i] = target_angles_[i].load();
        if (angles[i] != applied_angles_[i])
            changed = true;
    }

    if (changed) {
        go2_pawn_->SetJointAngles(angles);
        for (int i = 0; i < 12; ++i)
            applied_angles_[i] = angles[i];
    }

    // MuJoCo delta is meters in (forward, left, up). The rig's head points to
    // -X and its left is +Y, so map (x, y, z) to (-x, +y, z) in UE cm.
    float offset[3];
    bool offset_changed = false;
    for (int i = 0; i < 3; ++i) {
        offset[i] = base_offset_[i].load();
        if (offset[i] != applied_base_offset_[i])
            offset_changed = true;
    }
    if (offset_changed) {
        go2_pawn_->SetBaseOffset(FVector(-offset[0] * 100.0f, offset[1] * 100.0f, offset[2] * 100.0f));
        for (int i = 0; i < 3; ++i)
            applied_base_offset_[i] = offset[i];
    }
}

std::string Go2PawnSimApi::getRecordFileLine(bool is_header_line) const
{
    return PawnSimApi::getRecordFileLine(is_header_line);
}

void Go2PawnSimApi::setJointAngles(const std::vector<float>& angles)
{
    const size_t count = angles.size() < 12 ? angles.size() : 12;
    for (size_t i = 0; i < count; ++i)
        target_angles_[i].store(angles[i]);
}

std::vector<float> Go2PawnSimApi::getJointAngles() const
{
    std::vector<float> angles(12);
    for (int i = 0; i < 12; ++i)
        angles[i] = target_angles_[i].load();
    return angles;
}

void Go2PawnSimApi::setBaseOffset(const std::vector<float>& offset)
{
    const size_t count = offset.size() < 3 ? offset.size() : 3;
    for (size_t i = 0; i < count; ++i)
        base_offset_[i].store(offset[i]);
}
