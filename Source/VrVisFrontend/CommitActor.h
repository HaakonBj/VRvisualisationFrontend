// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "FArr.h"
#include "CommitActor.generated.h"

UCLASS()
class VRVISFRONTEND_API ACommitActor : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	USceneComponent* rootSceneComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	UStaticMeshComponent* sphereVisual;

	ACommitActor();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void Init(FArr data);
	// Called when the game starts
	virtual void BeginPlay() override;	
	// Called every frame
	virtual void Tick( float DeltaSeconds) override;
	UFUNCTION(BlueprintPure, Category = "Rest")
	int GetId();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetId(int newId);
	UFUNCTION(BlueprintPure, Category = "Rest")
	FString GetAuthor();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetAuthor(FString newAuthor);
	UFUNCTION(BlueprintPure, Category = "Rest")
	FString GetSha();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetSha(FString newSha);
	UFUNCTION(BlueprintPure, Category = "Rest")
	FString GetParentOne();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetParentOne(FString newParentOneSha);
	UFUNCTION(BlueprintPure, Category = "Rest")
	FString GetParentTwo();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetParentTwo(FString newParentTwoSha);
	UFUNCTION(BlueprintPure, Category = "Rest")
	FString GetDate();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetDate(FString newDate);

private:
	int id;
	FString sha;
	FString author;
	FString date;
	FString parentOne;
	FString parentTwo;
};
