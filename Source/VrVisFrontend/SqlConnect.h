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
	ASqlConnect();
	~ASqlConnect();
	virtual void BeginPlay() override;
	void InitDB();
	void AddCommit(FString id, FString sha, FString author, FString date, TArray<TSharedPtr<FJsonValue>> parents);
	std::vector<std::string> RetrieveCommitBySha(FString sha);
	std::vector<std::string> RetrieveCommitById(FString id);
	std::vector<std::vector<std::string>> RetrieveCommitsByAuthor(FString author);
	std::vector<std::string> SendQueryForSingleCommit(std::string statement);
	//Carefull with this one, you will have two git repo in the memory at the same time:
	std::vector<std::vector<std::string>> RetrieveWholeHistory();
	std::string CreateSQLTableStatement();
	std::vector<std::vector<std::string>> Query(const char* query);
	std::string FStringToString(FString in);
	FString StringToFString(std::string in);
};