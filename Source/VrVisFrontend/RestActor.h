// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "SqlConnect.h"
#include "CommitActor.h"
#include "ConnectionActor.h"
#include "RestActor.generated.h"

UCLASS()
class VRVISFRONTEND_API ARestActor : public AActor
{
	GENERATED_BODY()
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	USphereComponent* rootSphereComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	UStaticMeshComponent* coneVisual;
	FHttpModule* Http;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Rest")
	ASqlConnect* database;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	TArray<ACommitActor*> CommitArray;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	TArray<ACommitActor*> UnclaimedParentList;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	TArray<AConnectionActor*> ConnectionArray;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	TArray<AConnectionActor*> UnclaimedConnectionList;


	FVector newPosition;
	const int spaceIncrease = 15;
	TArray<int> indexesToTrackListToRemove;
	TArray<int> indexesToConnectionListToRemove;
	int lastIndex;
	int lastUsedConnectionIndex;
	ARestActor();
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void RetrieveDataFromMongoDB();
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void InitRestActor();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	FVector FindPosition(ACommitActor* current, ACommitActor* next);
	void UpdatePosition(ACommitActor* current, ACommitActor* next);
	void UpdateConnections();
	AConnectionActor * CreateConnectionActor();

	~ARestActor();
};
