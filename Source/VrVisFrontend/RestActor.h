// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "RestActor.generated.h"

UCLASS()
class VRVISFRONTEND_API ARestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	FHttpModule* Http;
	ARestActor();
	virtual void BeginPlay() override;
	void RetrieveDataFromMongoDB();
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
