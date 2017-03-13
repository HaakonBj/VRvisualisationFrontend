// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ConnectionActor.generated.h"

UCLASS()
class VRVISFRONTEND_API AConnectionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	USceneComponent* rootSceneComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	UStaticMeshComponent* cylinderVisual;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	bool horizontal;

	AConnectionActor();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void Init(bool verticalType);
	void setHorizontal();
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
};
