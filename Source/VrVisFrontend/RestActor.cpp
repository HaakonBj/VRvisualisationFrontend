// Fill out your copyright notice in the Description page of Project Settings.

#include "VrVisFrontend.h"
#include "RestActor.h"

// Sets default values
ARestActor::ARestActor()
{
	this->Http = &FHttpModule::Get();
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
	Request->SetURL("http://localhost:4000/vrVis");
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
		//Deserialize the json data given Reader and the actual object to deserialize
		if (FJsonSerializer::Deserialize(Reader, JsonParsed)) {
			TSharedPtr<FJsonObject> ParsedResponseObject = JsonParsed->AsArray()[0]->AsObject();
			FString id = ParsedResponseObject->GetStringField("_id");
			FString name = ParsedResponseObject->GetStringField("name");
			FString note = ParsedResponseObject->GetStringField("note");
			bool completed = ParsedResponseObject->GetBoolField("completed");
			FString date = ParsedResponseObject->GetStringField("updated_at");
			//TODO: Do stuff with the data
		} else {
			UE_LOG(LogTemp, Error, TEXT("Parsing of json data failed"));
		}
	} else {
		UE_LOG(LogTemp, Error, TEXT("The call to the backend failed! Response code is %d"), Response->GetResponseCode());
	}
}