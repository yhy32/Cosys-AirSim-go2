#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "PawnEvents.h"
#include "PIPCamera.h"
#include "common/common_utils/UniqueValueMap.hpp"

#include "Go2Pawn.generated.h"

class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class AIRSIM_API AGo2Pawn : public APawn
{
    GENERATED_BODY()

public:
    AGo2Pawn();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved,
                           FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

    void initializeForBeginPlay();

    const common_utils::UniqueValueMap<std::string, APIPCamera*> getCameras() const;
    PawnEvents* getPawnEvents()
    {
        return &pawn_events_;
    }

    void SetJointAngles(const TArray<float>& angles);
    TArray<float> GetJointAngles() const;
    void SetBaseOffset(const FVector& offset_cm);

    UPROPERTY(EditAnywhere, Category = "Go2")
    FString MeshFolder = TEXT("/AirSim/VehicleAdv/Go2Asset/parts");

    UPROPERTY(EditAnywhere, Category = "Go2")
    float BaseHeightCm = 27.0f;

    UPROPERTY(EditAnywhere, Category = "Go2")
    float ModelYawOffsetDeg = 180.0f;

    UPROPERTY(EditAnywhere, Category = "Go2")
    FVector CameraOffset = FVector(0, 0, 50);

    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* BaseMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* FLHipMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* FRHipMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* RLHipMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* RRHipMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* FLThighMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* FRThighMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* RLThighMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* RRThighMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* FLCalfMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* FRCalfMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* RLCalfMeshAsset = nullptr;
    UPROPERTY(EditAnywhere, Category = "Go2|Meshes") UStaticMesh* RRCalfMeshAsset = nullptr;

private:
    UStaticMeshComponent* createPart(const FName& name, USceneComponent* parent, const FVector& location_cm);
    void loadDefaultMeshes();
    void applyMeshes();
    void applyJointAngles();

    UPROPERTY() UStaticMeshComponent* base_mesh_;
    UPROPERTY() USceneComponent* leg_root_;
    UPROPERTY() UStaticMeshComponent* FL_hip_;
    UPROPERTY() UStaticMeshComponent* FR_hip_;
    UPROPERTY() UStaticMeshComponent* RL_hip_;
    UPROPERTY() UStaticMeshComponent* RR_hip_;
    UPROPERTY() UStaticMeshComponent* FL_thigh_;
    UPROPERTY() UStaticMeshComponent* FR_thigh_;
    UPROPERTY() UStaticMeshComponent* RL_thigh_;
    UPROPERTY() UStaticMeshComponent* RR_thigh_;
    UPROPERTY() UStaticMeshComponent* FL_calf_;
    UPROPERTY() UStaticMeshComponent* FR_calf_;
    UPROPERTY() UStaticMeshComponent* RL_calf_;
    UPROPERTY() UStaticMeshComponent* RR_calf_;

    UPROPERTY() USceneComponent* camera_base_;
    UPROPERTY() APIPCamera* camera_fpv_;
    UPROPERTY() UClass* pip_camera_class_;

    PawnEvents pawn_events_;
    TArray<float> joint_angles_;
    FVector spawn_location_ = FVector::ZeroVector;
};
