// Fill out your copyright notice in the Description page of Project Settings.

#include "VrVisFrontend.h"
#include "SqlConnect.h"


// Sets default values
ASqlConnect::ASqlConnect()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	sqlite3_open(":memory:", &this->db);
	UE_LOG(LogTemp, Log, TEXT("Did not fail!!!!"));

}

ASqlConnect::~ASqlConnect()
{
	sqlite3_close(this->db);
	this->db = nullptr;
}

// Called when the game starts or when spawned
void ASqlConnect::BeginPlay()
{
	Super::BeginPlay();
}

void ASqlConnect::AddCommit(FString id, FString sha, FString date, TArray<TSharedPtr<FJsonValue>> parents)
{
	//FString test = parents[0]-> ;
	//FJsonObject object = parents[0]->AsObject();
	auto test = parents[0]->AsObject();
	FString pID = test->GetStringField("_id");
}

