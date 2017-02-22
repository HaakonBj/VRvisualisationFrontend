// Fill out your copyright notice in the Description page of Project Settings.

#include "VrVisFrontend.h"
#include "SqlConnect.h"


// Sets default values
ASqlConnect::ASqlConnect()
{
	this->db = nullptr;
	this->InitDB();
}




ASqlConnect::~ASqlConnect()
{
	sqlite3_close(this->db);
	this->db = nullptr;
}

void ASqlConnect::InitDB() {
	int response;
	//create memory database
	response = sqlite3_open(":memory:", &this->db);
	if (response) {
		UE_LOG(LogTemp, Error, TEXT("Failed creating database %s \n"), sqlite3_errmsg(db));
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Created database"));
	}
	//Create Table in database


	this->Query(this->CreateSQLTableStatement().c_str());
}

// Called when the game starts or when spawned
void ASqlConnect::BeginPlay()
{
	Super::BeginPlay();
}

//TODO: consider adding a parentCount to the backend database
//TODO: convert the stuff straight to char *
void ASqlConnect::AddCommit(FString id, FString sha, FString date, TArray<TSharedPtr<FJsonValue>> parents) {
	//TODO: this really need to change, incase someone has more than 2 parents...and the table
	const int maxParents = 2;
	std::string insertStatement = "INSERT INTO HISTORY VALUES('"
		+ FStringToString(id) + "','"
		+ FStringToString(sha) + "','"
		+ FStringToString(date);

	for (int i = 0; i < parents.Num(); i++)
	{
		auto parent = parents[i]->AsObject();
		FString temp = parent->GetStringField("sha");
		std::string parentSha = FStringToString(temp);
		insertStatement += "','";
		insertStatement.append(parentSha);
	}

	if (parents.Num() < maxParents)
	{
		for (int i = maxParents - parents.Num(); i > 0; i--)
		{
			insertStatement += "','NULL";
		}
	}
	insertStatement.append("');");

	this->Query(insertStatement.c_str());
}

void ASqlConnect::RetrieveCommitBySha(FString sha) {
	std::string retrieveStatement = 
		"SELECT ID, AUTHOR, COMMITDATE, PARENTONE, PARENTTWO FROM HISTORY WHERE sha = "+ this->FStringToString(sha) + ";";
	this->Query(retrieveStatement.c_str());
}


//TODO handle more than 2 parents:
std::string ASqlConnect::CreateSQLTableStatement() {
	std::string sqlCreateTable = "CREATE TABLE HISTORY(" \
		"ID INT PRIMARY KEY		NOT NULL,"	\
		"AUTHOR			TEXT	NOT NULL,"	\
		"COMMITDATE		TEXT	NOT NULL,"	\
		"PARENTONE		TEXT			,"	\
		"PARENTTWO		TEXT	);";

	return sqlCreateTable;
}


//http://www.dreamincode.net/forums/topic/122300-sqlite-in-c/

std::vector<std::vector<std::string>> ASqlConnect::Query(const char * query)
{
	sqlite3_stmt *statement;
	std::vector<std::vector<std::string>> results;

	if (sqlite3_prepare_v2(this->db, query, -1, &statement, 0) == SQLITE_OK) {
		int cols = sqlite3_column_count(statement);
		int result = 0;
//TODO: check if you can use result as condition in while
		while (true)
		{
			result = sqlite3_step(statement);
			if (result == SQLITE_ROW)
			{
				std::vector<std::string> values;
				for (int col = 0; col < cols; col++)
				{
					std::string val;
					char* ptr = (char*)sqlite3_column_text(statement, col);
					if (ptr)
					{
						val = ptr;
					} else {
						val = "";
					}
					values.push_back(val);
				}
				results.push_back(values);
			}
			else
			{
				break;
			}
		}
		sqlite3_finalize(statement);
	}
	std::string error = sqlite3_errmsg(this->db);
	if (error != "not an error") { 
		FString errorString = StringToFString(error);
		UE_LOG(LogTemp, Error, TEXT("Failed creating database %s \n"), *errorString);
	}
	return results;
}

std::string ASqlConnect::FStringToString(FString in) {
	return TCHAR_TO_UTF8(*in);
}

FString ASqlConnect::StringToFString(std::string in) {
	return FString(in.c_str());
}