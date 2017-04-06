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
	this->newPosition = { 0, 0, 200 };
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
	if (bWasSuccessful) {
		//pointer for the parsed json data
		TSharedPtr<FJsonValue> JsonParsed;
		//reader pointer to read the json data from response
		TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
		//Deserialize the json data given Reader and the actual object to deserialize
		if (FJsonSerializer::Deserialize(Reader, JsonParsed)) {
			TArray<TSharedPtr<FJsonValue>> allCommits = JsonParsed->AsArray();
			for (auto iter : allCommits) {
				TSharedPtr<FJsonObject> commit = iter->AsObject();
				FString id = commit->GetStringField("_id");
				FString sha = commit->GetStringField("sha");
				FString author = commit->GetStringField("author");
				FString date = commit->GetStringField("commitDate");
				TArray<TSharedPtr<FJsonValue>> ParrentArray = commit->GetArrayField("parents");
				this->database->AddCommit(id, sha, author, date, ParrentArray);
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

void ARestActor::FindMaxAmountOfTracks(ACommitActor* current, ACommitActor* next) {
	this->indexesToParentListToRemove.Empty();
	if (current->GetParentTwo() != "NULL" && next->GetParentTwo() == "NULL") {
		bool hasParent = false;
		for (int i = 0; i < this->UnclaimedParentList.Num(); i++) {
			if (next->GetSha() == this->UnclaimedParentList[i]->GetParentOne()) {
				this->UnclaimedParentList[i] = next;
				hasParent = true;
			}
		}
		if (!hasParent) {
			this->UnclaimedParentList.Add(next);
			this->currentTrackCounter++;
		}
	} else {
		for (int i = 0; i < this->UnclaimedParentList.Num(); i++) {
			if (next->GetSha() == this->UnclaimedParentList[i]->GetParentOne()) {
				this->indexesToParentListToRemove.Add(i);
			}
		}

		if (this->indexesToParentListToRemove.Num() > 0) {
			this->indexToBeReplaced = this->indexesToParentListToRemove[0];
			for (int i = this->indexesToParentListToRemove.Num() - 1; i > 0; i--) {
				this->UnclaimedParentList.RemoveAt(this->indexesToParentListToRemove[i]);
				this->currentTrackCounter--;
			}
			this->UnclaimedParentList[this->indexToBeReplaced] = next;
		}
		else {
			this->UnclaimedParentList.Add(next);
			this->currentTrackCounter++;
		}
	}
	if (this->currentTrackCounter > this->maxAmountOfTracksCounter) {
		this->maxAmountOfTracksCounter = this->currentTrackCounter;
	}
}

FVector ARestActor::FindPosition(ACommitActor* current, ACommitActor* next) {
	this->indexesToParentListToRemove.Empty();
	float numberOfTracksBetween;
	float stepDegree;
	int index = 0;

	//Case where current has 2 parents and next has 1
	if (current->GetParentTwo() != "NULL" && next->GetParentTwo() == "NULL") {
		//Handles the case where the next commit is a parent of an existing commit:
		bool hasParent = false;
		for (int i = 0; i < this->UnclaimedParentList.Num(); i++) {
			if (next->GetSha() == this->UnclaimedParentList[i]->GetParentOne()) {
				this->UnclaimedParentList[i] = next;
				this->newPosition.Y = i * this->spaceIncrease;
				hasParent = true;
				index = i;
			}
		}
		//Handles the case where the next commit is NOT a parent of an existing commit:
		if (!hasParent) {
			this->newPosition.Y = this->UnclaimedParentList.Num() * this->spaceIncrease;
			index = this->UnclaimedParentList.Add(next);
		}
		//is this check not the same as hasParent? Can I just use hasParent?
		if (this->UnclaimedParentList.Find(current, this->indexToBeReplaced)) {
			numberOfTracksBetween = index - this->indexToBeReplaced;
			stepDegree = this->quarterRotation / numberOfTracksBetween;
			AConnectionActor* conActor = this->CreateConnectionActor(current->GetActorLocation(), numberOfTracksBetween, this->baseRotationForMergeConnection + stepDegree);
			this->ConnectionArray.Add(conActor);
		} else {
			UE_LOG(LogTemp, Warning, TEXT("Could not find commit %s"), *current->GetSha());
		}
	} else {
		this->UpdatePosition(current, next);
	}
	//Always decrease the z direction (down)
	this->newPosition.Z -= this->spaceIncrease;
	this->UpdateConnections(current, next);
	return this->newPosition;
}

//Handles the position update for most parent cases when positioning.
//See RestActor::FindPosition for the only case that does not work with this
void ARestActor::UpdatePosition(ACommitActor* current, ACommitActor* next) {
	int numberOfTracksBetween;
	float stepDegree;
	AConnectionActor* conActor;

	for (int i = 0; i < this->UnclaimedParentList.Num(); i++) {
		if (next->GetSha() == this->UnclaimedParentList[i]->GetParentOne()) {
			this->indexesToParentListToRemove.Add(i);
		}
	}

	if (this->indexesToParentListToRemove.Num() > 0) {
		this->indexToBeReplaced = this->indexesToParentListToRemove[0];
		for (int i = this->indexesToParentListToRemove.Num() - 1; i > 0; i--) {
			this->UnclaimedParentList.RemoveAt(this->indexesToParentListToRemove[i]);
			//This is where you can rotate around the z axis to create the circle
			//Spawn horizontal branch connectors
			this->SpawnHorizontalBranchConnection(this->indexesToParentListToRemove[i]);
		}
		this->UnclaimedParentList[this->indexToBeReplaced] = next;
		this->newPosition.Y = this->indexToBeReplaced * this->spaceIncrease;

		//Create certain merge connections for cases where current and next has 2 parents and next has branches from it
		int index;
		if (this->UnclaimedParentList.Find(current, index) && current->GetParentTwo() == next->GetSha()) {
			numberOfTracksBetween = this->indexToBeReplaced - index;
			stepDegree = this->quarterRotation / numberOfTracksBetween;
			conActor = this->CreateConnectionActor(current->GetActorLocation(), numberOfTracksBetween, this->baseRotationForMergeConnection + stepDegree);
			this->ConnectionArray.Add(conActor);
		}
	} else {
		this->newPosition.Y = this->UnclaimedParentList.Num() * spaceIncrease;
		int index = this->UnclaimedParentList.Add(next);
		//Creates certain merge connections where no branches are comming from next (e.g. case 1 & 1, 1 & 2 and 2 & 2):
		if (this->UnclaimedParentList.Find(current, this->indexToBeReplaced)) {
			numberOfTracksBetween = index - this->indexToBeReplaced;
			stepDegree = this->quarterRotation / numberOfTracksBetween;
			conActor = this->CreateConnectionActor(current->GetActorLocation(), numberOfTracksBetween, this->baseRotationForMergeConnection + stepDegree);
			this->ConnectionArray.Add(conActor);
		} else {
			UE_LOG(LogTemp, Warning, TEXT("Could not find commit %s"), *current->GetSha());
		}
	}
}
//TODO pass by reference instead
void ARestActor::UpdateConnections(ACommitActor* current, ACommitActor* next) {
	AConnectionActor* conActor;
	FVector position;

	if (this->lastUsedConnectionIndex == this->UnclaimedParentList.Num() - 1) {
		//Commit was added in existing tracklist entry
		this->ScaleVerticalConnections(0);
	} else if (this->lastUsedConnectionIndex < this->UnclaimedParentList.Num() - 1) {
		this->CreateVerticalConnection(this->newPosition);
		this->ScaleVerticalConnections(this->lastUsedConnectionIndex + 1);
		this->lastUsedConnectionIndex++;
	} else if (this->lastUsedConnectionIndex > this->UnclaimedParentList.Num() - 1) {
		//Commit was removed from tracklist
		bool finished = false;
		int p = this->UnclaimedConnectionList.Num() - 1;
		int qIndex;
		if (this->indexesToParentListToRemove.Num() > this->UnclaimedConnectionList.Num()) {
			qIndex = this->UnclaimedConnectionList.Num() - 1;
		} else {
			qIndex = this->indexesToParentListToRemove.Num() - 1;
		}
		int q = this->indexesToParentListToRemove[qIndex];
		int zScale;
		while (!finished) {
			if (qIndex == 0) {
				finished = true;
			} else if (p <= q) {
				this->UnclaimedConnectionList.RemoveAt(p);
				p--;
				qIndex--;
				q = this->indexesToParentListToRemove[qIndex];
			} else {
				//create horizontal
				position = current->GetActorLocation();
				position.Y = p * this->spaceIncrease;
				position.Z -= this->spaceIncrease / 2;
				zScale = qIndex;
				float stepDegree = this->quarterRotation / qIndex;
				conActor = this->CreateConnectionActor(position, zScale, this->baseRotationForBranchConnection - stepDegree);
				this->ConnectionArray.Add(conActor);
				p--;
			}
		}

		int runTo = this->UnclaimedConnectionList.Num();
		int clearFrom = this->indexesToParentListToRemove[1];
		this->UnclaimedConnectionList.RemoveAt(clearFrom, runTo - clearFrom);

		for (int i = clearFrom; i < runTo; i++) {
			position = current->GetActorLocation();
			position.Y = i * this->spaceIncrease;
			position.Z -= this->spaceIncrease;
			conActor = this->CreateAndReturnVerticalConnection(position);
			this->UnclaimedConnectionList.Add(conActor);
			this->ConnectionArray.Add(conActor);
		}

		this->ScaleVerticalConnections(this->indexesToParentListToRemove[1]);
		this->lastUsedConnectionIndex = this->UnclaimedConnectionList.Num() - 1;
	} else {
		this->lastUsedConnectionIndex--;
		this->ScaleVerticalConnections(0);
	}
}

AConnectionActor * ARestActor::CreateConnectionActor(FVector conPosition, int zScale, float degreesToRotate) {
	AConnectionActor* conActor = this->GetWorld()->SpawnActor<AConnectionActor>();
	conActor->SetActorLocation(conPosition);
	conActor->setHorizontal();
	FRotator rotator = conActor->GetActorRotation();
	rotator.Roll += degreesToRotate;
	conActor->SetActorRotation(rotator);
	conActor->SetActorScale3D(FVector(1, 1, zScale));
//	this->CheckIfToSetActorHidden(conActor);
	return conActor;
}

void ARestActor::CreateVerticalConnection(FVector position) {
	AConnectionActor* conActor = this->GetWorld()->SpawnActor<AConnectionActor>();
	position.Z += this->spaceIncrease / 2;
	conActor->SetActorLocation(position);
	FRotator rotator = conActor->GetActorRotation();
	rotator.Roll += this->baseRotationForVerticalConnection;
	conActor->SetActorRotation(rotator);
	FVector scale = conActor->GetActorScale();
	scale.Z = scale.Z;
	conActor->SetActorScale3D(scale);
//	this->CheckIfToSetActorHidden(conActor);
	this->UnclaimedConnectionList.Add(conActor);
	this->ConnectionArray.Add(conActor);
}

AConnectionActor* ARestActor::CreateAndReturnVerticalConnection(FVector position) {
	AConnectionActor* conActor = this->GetWorld()->SpawnActor<AConnectionActor>();
	conActor->SetActorLocation(position);
	FRotator rotator = conActor->GetActorRotation();
	rotator.Roll += this->baseRotationForVerticalConnection;
	conActor->SetActorRotation(rotator);
	FVector scale = conActor->GetActorScale();
	scale.Z = scale.Z / 2.0f;
	conActor->SetActorScale3D(scale);
//	this->CheckIfToSetActorHidden(conActor);
	return conActor;
}

void ARestActor::ScaleVerticalConnections(int scaleToIndex) {
	FVector scale;
	if (scaleToIndex != 0) {
		for (int i = 0; i < scaleToIndex; i++) {
			scale = this->UnclaimedConnectionList[i]->GetActorScale();
			scale.Z += 1;
			this->UnclaimedConnectionList[i]->SetActorScale3D(scale);
		}
	} else {
		for (auto * connection : this->UnclaimedConnectionList) {
			scale = connection->GetActorScale();
			scale.Z += 1;
			connection->SetActorScale3D(scale);
		}
	}
}

void ARestActor::SpawnHorizontalBranchConnection(int currentIndex) {
	int numberOfTracksBetween = currentIndex - this->indexToBeReplaced;
	float stepDegree = this->quarterRotation / numberOfTracksBetween;
	FVector conPosition = this->newPosition;
	conPosition.Y = currentIndex * this->spaceIncrease;
	conPosition.Z -= this->spaceIncrease / 2;
	AConnectionActor* conActor = this->CreateConnectionActor(conPosition, numberOfTracksBetween, this->baseRotationForBranchConnection - stepDegree);
//	this->CheckIfToSetActorHidden(conActor);
	this->ConnectionArray.Add(conActor);
}

void ARestActor::SetFloorActorReference(AStaticMeshActor* floorMesh) {
	this->floor = floorMesh;
}

void ARestActor::CheckIfToSetActorHidden(AActor * actorToBeHidden) {
	bool hidden;
	if (actorToBeHidden->GetActorLocation().Z < this->floor->GetActorLocation().Z) {
		hidden = true;
	} else {
		hidden = false;
	}
	actorToBeHidden->SetActorHiddenInGame(hidden);
}