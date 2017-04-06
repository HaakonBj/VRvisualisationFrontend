// Fill out your copyright notice in the Description page of Project Settings.

#include "VrVisFrontend.h"
#include "ConnectionActor.h"


// Sets default values
AConnectionActor::AConnectionActor() {
	this->rootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	this->RootComponent = this->rootSceneComponent;
	PrimaryActorTick.bCanEverTick = true;
	this->cylinderVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConnectionVisualRep"));
	this->cylinderVisual->SetupAttachment(rootSceneComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Cylinder.Shape_Cylinder"));
	if (CylinderVisualAsset.Succeeded()) {
		cylinderVisual->SetStaticMesh(CylinderVisualAsset.Object);
		cylinderVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		cylinderVisual->SetWorldScale3D(FVector(0.02373, 1.0, 0.15));
	} else {
		UE_LOG(LogTemp, Error, TEXT("Failed loading Mesh for Rest Actor root mesh component!"));
	}
	this->cylinderVisual->SetMobility(EComponentMobility::Movable);
	this->RootComponent->SetMobility(EComponentMobility::Movable);
	this->DisableComponentsSimulatePhysics();
	this->horizontal = false;
}

void AConnectionActor::Init(bool verticalType) {
//	FRotator rotator = FRotator(0, 90, 0);
//	if (verticalType) {
//		this->cylinderVisual->SetRelativeRotation(rotator);
//	} else {
//		//Create horizontal connection --
//	}
}

void AConnectionActor::setHorizontal() {
	this->horizontal = true;
}

void AConnectionActor::SetMergeConnection() {
	/*float cylinderRadius;
	float cylinderHalfHeight;
	this->sceneComponentForMergeComponent = NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("PlacementRepresentationForMergeAddition"));
	this->sceneComponentForMergeComponent->SetMobility(EComponentMobility::Movable);
	this->sceneComponentForMergeComponent->RegisterComponent();
	this->cylinderVisual->CalcBoundingCylinder(cylinderRadius, cylinderHalfHeight);
	this->sceneComponentForMergeComponent->AttachToComponent(this->rootSceneComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	FVector pos = this->sceneComponentForMergeComponent->GetComponentLocation();
	pos.Y += cylinderRadius * 2;
	pos.Z -= cylinderHalfHeight;
	this->sceneComponentForMergeComponent->SetWorldLocation(pos);
	this->cylinderVisualMergeAddition = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("ConnectionVisualRepMergeAddition"));
	UStaticMesh* mesh = this->LoadMeshFromPath(TEXT("/Game/StarterContent/Shapes/Shape_Cylinder.Shape_Cylinder"));
	this->cylinderVisualMergeAddition->SetStaticMesh(mesh);
	this->cylinderVisualMergeAddition->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	this->cylinderVisualMergeAddition->SetWorldScale3D(FVector(0.02373, 0.021641, 0.151469));
	this->cylinderVisualMergeAddition->SetMobility(EComponentMobility::Movable);
	this->cylinderVisualMergeAddition->RegisterComponent();
	this->cylinderVisualMergeAddition->K2_AttachToComponent(this->sceneComponentForMergeComponent, NAME_None, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld,EAttachmentRule::KeepRelative,false);*/
}

// Called when the game starts or when spawned
void AConnectionActor::BeginPlay() {
	Super::BeginPlay();
}

// Called every frame
void AConnectionActor::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}