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
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds) override;
	UFUNCTION(BlueprintPure, Category = "Rest")
	int GetId();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetId(int newId);
	UFUNCTION(BlueprintPure, Category = "Rest")
	FString GetAuthor() const;
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetAuthor(FString newAuthor);
	UFUNCTION(BlueprintPure, Category = "Rest")
	FString GetSha() const;
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetSha(FString newSha);
	UFUNCTION(BlueprintPure, Category = "Rest")
	FString GetParentOne() const;
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetParentOne(FString newParentOneSha);
	UFUNCTION(BlueprintPure, Category = "Rest")
	FString GetParentTwo() const;
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetParentTwo(FString newParentTwoSha);
	UFUNCTION(BlueprintPure, Category = "Rest")
	FString GetDate() const;
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void SetDate(FString newDate);

private:
	UPROPERTY(VisibleAnywhere, Category = "Rest")
	int id;
	UPROPERTY(VisibleAnywhere, Category = "Rest")
	FString sha;
	UPROPERTY(VisibleAnywhere, Category = "Rest")
	FString author;
	UPROPERTY(VisibleAnywhere, Category = "Rest")
	FString date;
	UPROPERTY(VisibleAnywhere, Category = "Rest")
	FString parentOne;
	UPROPERTY(VisibleAnywhere, Category = "Rest")
	FString parentTwo;
};
