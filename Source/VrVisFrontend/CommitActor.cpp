// Fill out your copyright notice in the Description page of Project Settings.

#include "VrVisFrontend.h"
#include "CommitActor.h"


//Sets default values for this component's properties
ACommitActor::ACommitActor() {
	this->DisableComponentsSimulatePhysics(); //possibly use actor->GetRootComponent()->SetSimulatePhysics( false ); in component
	this->rootSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	this->RootComponent = this->rootSphereComponent;
	this->rootSphereComponent->InitSphereRadius(5.0f);
	this->sphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	this->sphereVisual->SetupAttachment(rootSphereComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	if (SphereVisualAsset.Succeeded()) {
		sphereVisual->SetStaticMesh(SphereVisualAsset.Object);
		sphereVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
		sphereVisual->SetWorldScale3D(FVector(0.8f));
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed loading Mesh for Rest Actor root mesh component!"));
	}
	PrimaryActorTick.bCanEverTick = true;
}

//TODO: check if have to turn on ticks for parent actor e.g. RestActor
void ACommitActor::Init(FArr data) {
	this->id = FCString::Atoi(*data.arr[0]);
	this->sha = data.arr[1];
	this->author = data.arr[2];
	this->date = data.arr[3];
	this->parentOne = data.arr[4];
	this->parentTwo = data.arr[5];
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts
void ACommitActor::BeginPlay() {
	Super::BeginPlay();
}

// Called every frame
void ACommitActor::Tick( float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

int ACommitActor::GetId() {
	return this->id;
}

void ACommitActor::SetId(int newId) {
	this->id = newId;
}

FString ACommitActor::GetAuthor() {
	return this->author;
}

void ACommitActor::SetAuthor(FString newAuthor) {
	this->author = newAuthor;
}

FString ACommitActor::GetSha() {
	return this->sha;
}

void ACommitActor::SetSha(FString newSha) {
	this->sha = newSha;
}

FString ACommitActor::GetParentOne() {
	return this->parentOne;
}

void ACommitActor::SetParentOne(FString newParentOneSha) {
	this->parentOne = newParentOneSha;
}

FString ACommitActor::GetParentTwo() {
	return this->parentTwo;
}

void ACommitActor::SetParentTwo(FString newParentTwoSha) {
	this->parentTwo = newParentTwoSha;
}

FString ACommitActor::GetDate() {
	return this->date;
}

void ACommitActor::SetDate(FString newDate) {
	this->date = newDate;
}