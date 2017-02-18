// Fill out your copyright notice in the Description page of Project Settings.

#include "VrVisFrontend.h"
#include "RestActor.h"

// Sets default values
ARestActor::ARestActor()
{
	this->Http = &FHttpModule::Get();
	this->db = NewObject<ASqlConnect>();
	//this->dbActor = new ASqlConnect
}

// Called when the game starts or when spawned
void ARestActor::BeginPlay()
{
	RetrieveDataFromMongoDB();
	Super::BeginPlay();
}

//Http call
void ARestActor::RetrieveDataFromMongoDB() {
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ARestActor::OnResponseReceived);
	//TODO: Create proper routing in the backend and fix this:
	Request->SetURL("http://localhost:3000/GitHistory");
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->ProcessRequest();
}
//Callback
void ARestActor::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	if(bWasSuccessful){
		//pointer for the parsed json data
		TSharedPtr<FJsonValue> JsonParsed;
		//reader pointer to read the json data from response
		TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
		
		TArray<TSharedPtr<FJsonValue>> ParrentArray;
		//Deserialize the json data given Reader and the actual object to deserialize
		if (FJsonSerializer::Deserialize(Reader, JsonParsed)) {
			TSharedPtr<FJsonObject> ParsedResponseObject = JsonParsed->AsArray()[0]->AsObject();
			FString id = ParsedResponseObject->GetStringField("_id");
			FString sha = ParsedResponseObject->GetStringField("author");
			FString date = ParsedResponseObject->GetStringField("commitDate");
			
			ParrentArray = ParsedResponseObject->GetArrayField("parents");
			
			
			
			
			
			//TODO: Loop and save off the data to the memory database.
			this->db->AddCommit("", "", "", ParrentArray);
		} else {
			UE_LOG(LogTemp, Error, TEXT("Parsing of json data failed"));
		}
	} else {
		if (Response.IsValid())
		{
			FString responseCode = FString::FromInt(Response->GetResponseCode());
			UE_LOG(LogTemp, Error, TEXT("The call to the backend failed! Response code is %d"), *responseCode);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("The call to the backend failed! No response code was received!"));
		}
	}
}

ARestActor::~ARestActor()
{
	//UE4 gc's unreferenced objects, handling delete themselves.
	this->db = nullptr;
}
