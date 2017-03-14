// Fill out your copyright notice in the Description page of Project Settings.

#include "VrVisFrontend.h"
#include "RestActor.h"

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
	this->coneVisual->SetMobility(EComponentMobility::Movable);
	this->RootComponent->SetMobility(EComponentMobility::Movable);
	this->newPosition = {0, 0, 200};
	this->lastUsedConnectionIndex = 0;
	this->UnclaimedConnectionList.Reserve(2500);
	this->UnclaimedParentList.Reserve(2500);
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
	this->indexesToTrackListToRemove.Empty();
	//Case where current has 2 parents and next has 1
	if (current->GetParentTwo() != "NULL" && next->GetParentTwo() == "NULL") {
	//Handles the case where the next commit is a parent of an existing commit:
		bool hasParent = false;
		for (int i = 0; i < this->UnclaimedParentList.Num(); i++) {
			if (next->GetSha() == this->UnclaimedParentList[i]->GetParentOne()) {
				this->UnclaimedParentList[i] = next;
				this->newPosition.Y = i * this->spaceIncrease;
				hasParent = true;
			}
		}
		//Handles the case where the next commit is NOT a parent of an existing commit:
		if (!hasParent) {
			this->newPosition.Y = this->UnclaimedParentList.Num() * this->spaceIncrease;
			this->UnclaimedParentList.Add(next);
		}
	} else {
		this->UpdatePosition(current, next);
	}
	//Always decrease the z direction (down)
	this->newPosition.Z -= this->spaceIncrease;
	return this->newPosition;
}

//Handles the position update for most parent cases when positioning.
//See RestActor::FindPosition for the only case that does not work with this
void ARestActor::UpdatePosition(ACommitActor* current, ACommitActor* next) {
	int numberOfTracksBetween;
	float stepDegree;

	for (int i = 0; i < this->UnclaimedParentList.Num(); i++) {
		if (next->GetSha() == this->UnclaimedParentList[i]->GetParentOne()) {
			this->indexesToTrackListToRemove.Add(i);
		}
	}

	if (this->indexesToTrackListToRemove.Num() > 0) {
		this->lastIndex = this->indexesToTrackListToRemove[0];
		for (int i = this->indexesToTrackListToRemove.Num() - 1; i > 0; i--) {
			this->UnclaimedParentList.RemoveAt(this->indexesToTrackListToRemove[i]);
			//This is where you can rotate around the z axis to create the circle
			//Spawn horizontal branch connectors
			//TODO: maybe throw into function
			int currentIndex = this->indexesToTrackListToRemove[i];
			numberOfTracksBetween = currentIndex - this->lastIndex;
			stepDegree = 45 - (45 / numberOfTracksBetween);
			float yPos = this->indexesToTrackListToRemove[i] * this->spaceIncrease;
			FVector conPosition = this->newPosition;
			conPosition.Y = yPos;
			AConnectionActor* conActor = this->CreateConnectionActor(conPosition, numberOfTracksBetween, this->baseRotationForBranchConnection + stepDegree);
			this->ConnectionArray.Add(conActor);
		}
		this->UnclaimedParentList[this->lastIndex] = next;
		this->newPosition.Y = this->lastIndex * this->spaceIncrease;
	} else {
		this->newPosition.Y = this->UnclaimedParentList.Num() * spaceIncrease;
		int index = this->UnclaimedParentList.Add(next);
		//Creates certain merge connections:
		if (this->UnclaimedParentList.Find(current, this->lastIndex)) {
			numberOfTracksBetween = index - this->lastIndex;
			stepDegree = 45 / numberOfTracksBetween;
			AConnectionActor* conActor = this->CreateConnectionActor(current->GetActorLocation(), numberOfTracksBetween, this->baseRotationForMergeConnection + stepDegree);
			this->ConnectionArray.Add(conActor);
		} else {
			UE_LOG(LogTemp, Warning, TEXT("Could not find commit %s"), *current->GetSha());
		}
		UE_LOG(LogTemp, Warning, TEXT("Current was %s"), *current->GetSha());
	}
}

void ARestActor::UpdateConnections() {
	FVector currentPosition;
	FVector currentScale;
	for (int i = 0; i < this->UnclaimedConnectionList.Num(); i++) {
		if (i != this->lastUsedConnectionIndex) {
			currentPosition = this->UnclaimedConnectionList[i]->GetActorLocation();
			currentPosition.Z = currentPosition.Z + (this->spaceIncrease / 2);
			this->UnclaimedConnectionList[i]->SetActorLocation(currentPosition);
			currentScale = this->UnclaimedConnectionList[i]->GetActorScale3D();
			currentScale.Z += 0.5;
			this->UnclaimedConnectionList[i]->SetActorScale3D(currentScale);
		}
	}
}
//TODO: When finished check if you need to return the connection actors from CreateConnectionActor() functions
AConnectionActor * ARestActor::CreateConnectionActor(FVector conPosition, int zScale, float degreesToRotate) {
	AConnectionActor* connectionActor = this->GetWorld()->SpawnActor<AConnectionActor>();
	connectionActor->SetActorLocation(conPosition);
	connectionActor->setHorizontal();
	FRotator rotator = connectionActor->GetActorRotation();
	rotator.Roll += degreesToRotate;
	connectionActor->SetActorRotation(rotator);
	connectionActor->SetActorScale3D(FVector(1, 1, zScale));
	return connectionActor;
}