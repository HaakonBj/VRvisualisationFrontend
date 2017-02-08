// Fill out your copyright notice in the Description page of Project Settings.

#include "VrVisFrontend.h"
#include "SqlConnect.h"
#include "sqlite3.h"

// Sets default values
ASqlConnect::ASqlConnect()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	sqlite3 * db;
	sqlite3_open(":memory:", &db);
	//UE_LOG(LogTemp, Log, TEXT("Finished!!!!"));
	UE_LOG(LogTemp, Log, TEXT("Did not fail!!!!"));
	UE_LOG(LogTemp, Log, TEXT("What the fuck!!!!"));
	UE_LOG(LogTemp, Log, TEXT("What are you doing Unreal!!!!"));
}

// Called when the game starts or when spawned
void ASqlConnect::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASqlConnect::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

