#include "Go2Pawn.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

#include "AirBlueprintLib.h"
#include "common/Common.hpp"

// Joint names/angles follow the visual front (head at -X after the body yaw).
// role order: FL, FR, RL, RR -> physical component index (0=FL,1=FR,2=RL,3=RR)
static constexpr int kPhysFromRole[4] = { 3, 2, 1, 0 };

AGo2Pawn::AGo2Pawn()
{
    static ConstructorHelpers::FClassFinder<APIPCamera> pip_camera_class(TEXT("Blueprint'/AirSim/Blueprints/BP_PIPCamera'"));
    pip_camera_class_ = pip_camera_class.Succeeded() ? pip_camera_class.Class : nullptr;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    base_mesh_ = createPart(TEXT("Base"), RootComponent, FVector(0, 0, BaseHeightCm));
    base_mesh_->SetRelativeRotation(FRotator(0, ModelYawOffsetDeg, 0));

    // Legs are attached to their own root so the body/head can be rotated
    // independently of the leg layout.
    leg_root_ = CreateDefaultSubobject<USceneComponent>(TEXT("leg_root"));
    leg_root_->SetupAttachment(RootComponent);
    leg_root_->SetRelativeLocation(FVector(0, 0, BaseHeightCm));

    FL_hip_ = createPart(TEXT("FL_hip"), leg_root_, FVector(19.34, -4.65, 0));
    FR_hip_ = createPart(TEXT("FR_hip"), leg_root_, FVector(19.34, 4.65, 0));
    RL_hip_ = createPart(TEXT("RL_hip"), leg_root_, FVector(-19.34, -4.65, 0));
    RR_hip_ = createPart(TEXT("RR_hip"), leg_root_, FVector(-19.34, 4.65, 0));

    FL_thigh_ = createPart(TEXT("FL_thigh"), FL_hip_, FVector(0, -9.55, 0));
    FR_thigh_ = createPart(TEXT("FR_thigh"), FR_hip_, FVector(0, 9.55, 0));
    RL_thigh_ = createPart(TEXT("RL_thigh"), RL_hip_, FVector(0, -9.55, 0));
    RR_thigh_ = createPart(TEXT("RR_thigh"), RR_hip_, FVector(0, 9.55, 0));

    FL_calf_ = createPart(TEXT("FL_calf"), FL_thigh_, FVector(0, 0, -21.3));
    FR_calf_ = createPart(TEXT("FR_calf"), FR_thigh_, FVector(0, 0, -21.3));
    RL_calf_ = createPart(TEXT("RL_calf"), RL_thigh_, FVector(0, 0, -21.3));
    RR_calf_ = createPart(TEXT("RR_calf"), RR_thigh_, FVector(0, 0, -21.3));

    camera_base_ = CreateDefaultSubobject<USceneComponent>(TEXT("camera_base"));
    camera_base_->SetupAttachment(RootComponent);
    camera_base_->SetRelativeLocation(CameraOffset);

    joint_angles_.Init(0.0f, 12);
    for (int leg = 0; leg < 4; ++leg) {
        joint_angles_[leg * 3 + 1] = 0.9f;
        joint_angles_[leg * 3 + 2] = -1.8f;
    }
}

UStaticMeshComponent* AGo2Pawn::createPart(const FName& name, USceneComponent* parent, const FVector& location_cm)
{
    UStaticMeshComponent* part = CreateDefaultSubobject<UStaticMeshComponent>(name);
    part->SetupAttachment(parent);
    part->SetRelativeLocation(location_cm);
    part->SetMobility(EComponentMobility::Movable);
    part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    part->SetGenerateOverlapEvents(false);
    part->SetCastShadow(true);
    return part;
}

void AGo2Pawn::BeginPlay()
{
    Super::BeginPlay();
}

void AGo2Pawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    pawn_events_.getPawnTickSignal().emit(DeltaSeconds);
}

void AGo2Pawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (camera_fpv_ != nullptr) {
        camera_fpv_->Destroy();
        camera_fpv_ = nullptr;
    }
    Super::EndPlay(EndPlayReason);
}

void AGo2Pawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved,
                         FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    pawn_events_.getCollisionSignal().emit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
}

void AGo2Pawn::initializeForBeginPlay()
{
    base_mesh_->SetRelativeLocation(FVector(0, 0, BaseHeightCm));
    base_mesh_->SetRelativeRotation(FRotator(0, ModelYawOffsetDeg, 0));
    leg_root_->SetRelativeLocation(FVector(0, 0, BaseHeightCm));
    camera_base_->SetRelativeLocation(CameraOffset);

    loadDefaultMeshes();
    applyMeshes();

    // Snap to the ground below the spawn point (the AirSim NED origin is the
    // map's player start, which may be above the floor).
    {
        FHitResult hit;
        const FVector start = GetActorLocation() + FVector(0, 0, 100.0f);
        const FVector end = GetActorLocation() - FVector(0, 0, 5000.0f);
        FCollisionQueryParams params(SCENE_QUERY_STAT(Go2Ground), false, this);
        params.AddIgnoredActor(this);
        if (GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_WorldStatic, params)) {
            FVector loc = GetActorLocation();
            loc.Z = hit.ImpactPoint.Z;
            SetActorLocation(loc, false, nullptr, ETeleportType::TeleportPhysics);
        }
        spawn_location_ = GetActorLocation();
    }

    if (pip_camera_class_ != nullptr && camera_fpv_ == nullptr) {
        FActorSpawnParameters camera_spawn_params;
        camera_spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        camera_spawn_params.Name = FName(*(GetName() + "_camera_fpv"));

        camera_fpv_ = GetWorld()->SpawnActor<APIPCamera>(pip_camera_class_, FTransform::Identity, camera_spawn_params);
        if (camera_fpv_ != nullptr)
            camera_fpv_->AttachToComponent(camera_base_, FAttachmentTransformRules::KeepRelativeTransform);
    }

    applyJointAngles();
}

void AGo2Pawn::loadDefaultMeshes()
{
    if (MeshFolder.IsEmpty())
        return;

    const FString folder = MeshFolder.EndsWith(TEXT("/")) ? MeshFolder.LeftChop(1) : MeshFolder;

    auto load = [&folder](const TCHAR* name) -> UStaticMesh* {
        const FString path = FString::Printf(TEXT("%s/%s.%s"), *folder, name, name);
        UStaticMesh* mesh = LoadObject<UStaticMesh>(nullptr, *path);
        if (mesh == nullptr)
            UAirBlueprintLib::LogMessageString("Go2 mesh not found: ", TCHAR_TO_UTF8(*path), LogDebugLevel::Informational);
        return mesh;
    };

    if (BaseMeshAsset == nullptr) BaseMeshAsset = load(TEXT("base"));
    if (FLHipMeshAsset == nullptr) FLHipMeshAsset = load(TEXT("FL_hip"));
    if (FRHipMeshAsset == nullptr) FRHipMeshAsset = load(TEXT("FR_hip"));
    if (RLHipMeshAsset == nullptr) RLHipMeshAsset = load(TEXT("RL_hip"));
    if (RRHipMeshAsset == nullptr) RRHipMeshAsset = load(TEXT("RR_hip"));
    if (FLThighMeshAsset == nullptr) FLThighMeshAsset = load(TEXT("FL_thigh"));
    if (FRThighMeshAsset == nullptr) FRThighMeshAsset = load(TEXT("FR_thigh"));
    if (RLThighMeshAsset == nullptr) RLThighMeshAsset = load(TEXT("RL_thigh"));
    if (RRThighMeshAsset == nullptr) RRThighMeshAsset = load(TEXT("RR_thigh"));
    if (FLCalfMeshAsset == nullptr) FLCalfMeshAsset = load(TEXT("FL_calf"));
    if (FRCalfMeshAsset == nullptr) FRCalfMeshAsset = load(TEXT("FR_calf"));
    if (RLCalfMeshAsset == nullptr) RLCalfMeshAsset = load(TEXT("RL_calf"));
    if (RRCalfMeshAsset == nullptr) RRCalfMeshAsset = load(TEXT("RR_calf"));
}

void AGo2Pawn::applyMeshes()
{
    // Only assign when the component is still empty, so meshes assigned
    // directly on inherited components are not overwritten.
    auto assign = [](UStaticMeshComponent* part, UStaticMesh* mesh) {
        if (part != nullptr && mesh != nullptr && part->GetStaticMesh() == nullptr)
            part->SetStaticMesh(mesh);
    };

    assign(base_mesh_, BaseMeshAsset);
    assign(FL_hip_, FLHipMeshAsset);
    assign(FR_hip_, FRHipMeshAsset);
    assign(RL_hip_, RLHipMeshAsset);
    assign(RR_hip_, RRHipMeshAsset);
    assign(FL_thigh_, FLThighMeshAsset);
    assign(FR_thigh_, FRThighMeshAsset);
    assign(RL_thigh_, RLThighMeshAsset);
    assign(RR_thigh_, RRThighMeshAsset);
    assign(FL_calf_, FLCalfMeshAsset);
    assign(FR_calf_, FRCalfMeshAsset);
    assign(RL_calf_, RLCalfMeshAsset);
    assign(RR_calf_, RRCalfMeshAsset);
}

void AGo2Pawn::SetJointAngles(const TArray<float>& angles)
{
    if (angles.Num() < 12)
        return;

    joint_angles_.SetNumUninitialized(12);
    for (int i = 0; i < 12; ++i)
        joint_angles_[i] = angles[i];

    applyJointAngles();
}

TArray<float> AGo2Pawn::GetJointAngles() const
{
    return joint_angles_;
}

void AGo2Pawn::SetBaseOffset(const FVector& offset_cm)
{
    SetActorLocation(spawn_location_ + offset_cm, false, nullptr, ETeleportType::TeleportPhysics);
}

void AGo2Pawn::applyJointAngles()
{
    auto set_hinge = [](UStaticMeshComponent* part, float angle, const FVector& axis) {
        const float half = angle * 0.5f;
        const FQuat rotation(axis.X * FMath::Sin(half), axis.Y * FMath::Sin(half), axis.Z * FMath::Sin(half), FMath::Cos(half));
        part->SetRelativeRotation(rotation);
    };

    const FVector x_axis(1, 0, 0);
    const FVector y_axis(0, 1, 0);

    UStaticMeshComponent* hips[4] = { FL_hip_, FR_hip_, RL_hip_, RR_hip_ };
    UStaticMeshComponent* thighs[4] = { FL_thigh_, FR_thigh_, RL_thigh_, RR_thigh_ };
    UStaticMeshComponent* calves[4] = { FL_calf_, FR_calf_, RL_calf_, RR_calf_ };

    for (int role = 0; role < 4; ++role) {
        const int leg = kPhysFromRole[role];
        set_hinge(hips[leg], joint_angles_[role * 3 + 0], x_axis);
        set_hinge(thighs[leg], -joint_angles_[role * 3 + 1], y_axis);
        set_hinge(calves[leg], -joint_angles_[role * 3 + 2], y_axis);
    }
}

const common_utils::UniqueValueMap<std::string, APIPCamera*> AGo2Pawn::getCameras() const
{
    common_utils::UniqueValueMap<std::string, APIPCamera*> cameras;
    cameras.insert_or_assign("front_center", camera_fpv_);
    cameras.insert_or_assign("0", camera_fpv_);
    cameras.insert_or_assign("fpv", camera_fpv_);
    cameras.insert_or_assign("", camera_fpv_);
    cameras.insert_or_assign("back_center", camera_fpv_);
    cameras.insert_or_assign("bottom_center", camera_fpv_);
    return cameras;
}
