// Fill out your copyright notice in the Description page of Project Settings.

#include "VrVisFrontend.h"
#include "RestActor.h"

// Sets default values
ARestActor::ARestActor() {
	this->rootSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	this->RootComponent = this->rootSphereComponent;
	this->rootSphereComponent->InitSphereRadius(40.0f);
	this->coneVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	this->coneVisual->SetupAttachment(rootSphereComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Cone.Shape_Cone"));
	if (SphereVisualAsset.Succeeded()) {
		coneVisual->SetStaticMesh(SphereVisualAsset.Object);
		coneVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
		coneVisual->SetWorldScale3D(FVector(0.8f));
	} else {
		UE_LOG(LogTemp, Error, TEXT("Failed loading Mesh for Rest Actor root mesh component!"));
	}
	//this->DisableComponentsSimulatePhysics(); //possibly use actor->GetRootComponent()->SetSimulatePhysics( false ); in component
	this->indexCounter = 0;
	this->coneVisual->SetMobility(EComponentMobility::Movable);
	this->RootComponent->SetMobility(EComponentMobility::Movable);
	this->newPosition = { 0,0,200 };
}

void ARestActor::InitRestActor() {
	this->Http = &FHttpModule::Get();
	this->database = NewObject<ASqlConnect>();
	this->database->AddToRoot();
}

// Called when the game starts or when spawned
void ARestActor::BeginPlay() {
	Super::BeginPlay();
}

ARestActor::~ARestActor() {
	//UE4 gc's unreferenced objects, handling delete themselves.
	this->database = nullptr;
}

//Http call
void ARestActor::RetrieveDataFromMongoDB() {
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ARestActor::OnResponseReceived);
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
		//Deserialize the json data given Reader and the actual object to deserialize
		if (FJsonSerializer::Deserialize(Reader, JsonParsed)) {
			TArray<TSharedPtr<FJsonValue>> allCommits = JsonParsed->AsArray();
			for (auto iter: allCommits) {
				TSharedPtr<FJsonObject> commit = iter->AsObject();
				FString id = commit->GetStringField("_id");
				FString sha = commit->GetStringField("sha");
				FString author = commit->GetStringField("author");
				FString date = commit->GetStringField("commitDate");
				TArray<TSharedPtr<FJsonValue>> ParrentArray = commit->GetArrayField("parents");
				this->database->AddCommit(id , sha, author, date, ParrentArray);
			}
		} else {
			UE_LOG(LogTemp, Error, TEXT("Parsing of json data failed"));
		}
		//Set the database to know it has got data now
		this->database->dbGotData = true;
	} else {
		if (Response.IsValid()) {
			FString responseCode = FString::FromInt(Response->GetResponseCode());
			UE_LOG(LogTemp, Error, TEXT("The call to the backend failed! Response code is %d"), *responseCode);
		} else {
			UE_LOG(LogTemp, Error, TEXT("The call to the backend failed! No response code was received!"));
		}
	}
}


FVector ARestActor::FindPosition(ACommitActor* current, ACommitActor* next) {
	const int spaceIncrease = 15;
	
	if (current->GetParentTwo() == "NULL" && next->GetParentTwo() != "NULL") {


	} else if (current->GetParentTwo() == "NULL" && next->GetParentTwo() == "NULL") {


	} else if (current->GetParentTwo() != "NULL" && next->GetParentTwo() != "NULL") {
	

	} else if (current->GetParentTwo() != "NULL" && next->GetParentTwo() == "NULL") {


	}
	this->newPosition.Z -= spaceIncrease;
	return this->newPosition;
}


//TODO: Find a better and correct way
//FVector ARestActor::FindPosition(ACommitActor* current, ACommitActor* next) {
//	const int spaceIncrease = 15;
//
//	if (current->GetParentTwo() == "NULL" && next->GetParentTwo() != "NULL") {
//		if (this->list.Num() != 0) {
//			int index = this->list.Pop();
//			//TODO log this and find the error:
//			if (this->list.Num() != 0 )
//			{
//				if (this->list.Last() == index)
//				{
//					this->list.RemoveAt(this->list.Num() -1);
//				}
//			}
//			
//			if (index > this->CommitArray.Num()) {
//				UE_LOG(LogTemp, Warning, TEXT("Tried to access an index greater than the array size, this should never happen!"));
//			} else {
//				this->newPosition.Y = this->CommitArray[index]->GetActorLocation().Y;
//				//UE_LOG(LogTemp, Warning, TEXT("oldPosition for commit %d is %s"), index, *this->CommitArray[index]->GetActorLocation().ToString());
//				//UE_LOG(LogTemp, Warning, TEXT("newPosition for commit %d is %s"), next->GetId(), *this->newPosition.ToString());
//				UE_LOG(LogTemp, Warning, TEXT("list info: num: %d, last was: %d"), list.Num(), index);
//				UE_LOG(LogTemp, Warning, TEXT("In the list there is: %d, as the last element"), list.Last());
//			}
//		} else {
//			UE_LOG(LogTemp, Warning, TEXT("I just tried poping of an empty list. Current: %d . Next: %d"), current->GetId(), next->GetId());
//			this->newPosition.Y = this->newPosition.Y + spaceIncrease;
//		}
//		//TODO: check if you only need current->GetParentTwo() != "NULL" on this and the next if test:
//	} else if (current->GetParentTwo() != "NULL" && next->GetParentTwo() == "NULL") {
//		this->newPosition.Y = this->newPosition.Y - spaceIncrease;
//		this->list.Add(this->indexCounter);
//		//UE_LOG(LogTemp, Warning, TEXT("I just added to the list: %d. The size of the array is now: %d"), this->indexCounter, list.Num());
//	} else if (current->GetParentTwo() != "NULL" && next->GetParentTwo() != "NULL") {
//		this->newPosition.Y = this->newPosition.Y - spaceIncrease;
//		this->list.Add(this->indexCounter);
//	}
//	this->newPosition.Z = this->newPosition.Z - spaceIncrease;
//	if (this->newPosition.Y < -120){
//		UE_LOG(LogTemp, Warning, TEXT("I moved further than -120 in y axis, there is a mistake, my pos is %s: "),*this->newPosition.ToString());
//	}
//
//	this->indexCounter += 1;
//	return this->newPosition;
//}