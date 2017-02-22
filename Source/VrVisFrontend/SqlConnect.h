// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "sqlite3.h"
#include <string>
#include <vector>
#include "SqlConnect.generated.h"


UCLASS()
class VRVISFRONTEND_API ASqlConnect : public AActor
{
	GENERATED_BODY()
	
public:	
	
	sqlite3 * db;
	//TSharedPtr<sqlite3> db;
	// Sets default values for this actor's properties
	ASqlConnect();
	~ASqlConnect();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void InitDB();
	void AddCommit(FString id, FString sha, FString date, TArray<TSharedPtr<FJsonValue>> parents);
	void RetrieveCommitBySha(FString sha);
	std::string CreateSQLTableStatement();
	std::vector<std::vector<std::string>> Query(const char* query);
	std::string FStringToString(FString in);
	FString StringToFString(std::string in);
};