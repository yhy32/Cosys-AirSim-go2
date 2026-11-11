#pragma once

#include "api/VehicleApiBase.hpp"
#include "common/AirSimSettings.hpp"

namespace msr
{
namespace airlib
{
    // Minimal vehicle API for the rendering-only Go2. The real state comes
    // from MuJoCo, so this only satisfies the common AirSim vehicle API.
    class Go2Api : public VehicleApiBase
    {
    public:
        Go2Api(const AirSimSettings::VehicleSetting* vehicle_setting, const GeoPoint& home_geopoint)
            : vehicle_setting_(vehicle_setting)
            , home_geopoint_(home_geopoint)
        {
        }

        virtual void enableApiControl(bool is_enabled) override
        {
            api_control_enabled_ = is_enabled;
        }

        virtual bool isApiControlEnabled() const override
        {
            return api_control_enabled_;
        }

        virtual bool armDisarm(bool arm) override
        {
            unused(arm);
            return true;
        }

        virtual GeoPoint getHomeGeoPoint() const override
        {
            return home_geopoint_;
        }

        virtual const SensorCollection& getSensors() const override
        {
            return sensors_;
        }

    protected:
        virtual void resetImplementation() override
        {
        }

    private:
        bool api_control_enabled_ = false;
        const AirSimSettings::VehicleSetting* vehicle_setting_;
        GeoPoint home_geopoint_;
        SensorCollection sensors_;
    };
}
}
