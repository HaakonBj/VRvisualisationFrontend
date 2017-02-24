// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "sqlite3.h"
#include <string>
#include <vector>
#include "FArr.h"
#include "SqlConnect.generated.h"

UCLASS()
class VRVISFRONTEND_API ASqlConnect : public AActor
{
	GENERATED_BODY()
	
public:
	sqlite3 * db;
	ASqlConnect();
	~ASqlConnect();
	bool dbIsReady;
	UFUNCTION(BlueprintPure, Category = "SqlConnect")
	bool DatabaseIsReady();
	virtual void BeginPlay() override;
	void InitDB();
	void AddCommit(FString id, FString sha, FString author, FString date, TArray<TSharedPtr<FJsonValue>> parents);
	TArray<FString> RetrieveCommitBySha(FString sha);
	TArray<FString> RetrieveCommitById(FString id);
	TArray<FArr> RetrieveCommitsByAuthor(FString author);
	TArray<FArr> Query(const char* query);
	TArray<FString> SendQueryForSingleCommit(std::string statement);
	//Carefull with this one, you will have two git repo in the memory at the same time:
	UFUNCTION(BlueprintCallable, Category = "SqlConnect")
	TArray<FArr> RetrieveWholeHistory();
	std::string CreateSQLTableStatement();
	std::string FStringToString(FString in);
	FString StringToFString(std::string in);
	FString CharArrayToFString(char * in);
};