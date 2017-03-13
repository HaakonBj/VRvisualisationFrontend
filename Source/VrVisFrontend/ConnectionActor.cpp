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
		cylinderVisual->SetWorldScale3D(FVector(0.02373, 0.031641, 0.105469));
	}
	else {
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

// Called when the game starts or when spawned
void AConnectionActor::BeginPlay() {
	Super::BeginPlay();
}

// Called every frame
void AConnectionActor::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}