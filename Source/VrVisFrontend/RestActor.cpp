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
	this->newPosition = { 60, 0, 200 };
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

void ARestActor::SetRotationAmount() {
	this->rotationAmount = 360 / (this->maxAmountOfTracksCounter +1);
}

FVector ARestActor::FindPosition(ACommitActor* current, ACommitActor* next) {
	this->indexesToParentListToRemove.Empty();
	this->newlyCreatedConnectionIndexes.Empty();

	//Case where current has 2 parents and next has 1
	if (current->GetParentTwo() != "NULL" && next->GetParentTwo() == "NULL") {
		//Handles the case where the next commit is a parent of an existing commit:
		bool hasParent = false;
		for (int i = 0; i < this->UnclaimedParentList.Num(); i++) {
			if (this->UnclaimedParentList[i] != NULL && next->GetSha() == this->UnclaimedParentList[i]->GetParentOne()) {
				//Replacing commit in existing track, should not require changes to connections:
				this->UnclaimedParentList[i] = next;
				float angle = i * this->rotationAmount;
				float radian = UKismetMathLibrary::DegreesToRadians(angle);
				this->newPosition.Y = sin(radian) * this->spaceIncrease;
				this->newPosition.X = cos(radian) * this->spaceIncrease;
				hasParent = true;
			}
		}
		//Handles the case where the next commit is NOT a parent of an existing commit:
		if (!hasParent) {
			bool placed = false;
			//Adding new commit track:
			for (int i = 0; i < this->UnclaimedParentList.Num(); i++) {
				if (this->UnclaimedParentList[i] == NULL) {
					this->UnclaimedParentList[i] = next;
					placed = true;
					float angle = i * this->rotationAmount;
					float radian = UKismetMathLibrary::DegreesToRadians(angle);
					this->newPosition.Y = sin(radian) * this->spaceIncrease;
					this->newPosition.X = cos(radian) * this->spaceIncrease;
					this->UnclaimedConnectionList[i] = this->CreateAndReturnVerticalConnection();
					this->newlyCreatedConnectionIndexes.Add(i);
					break;
				}
			}
			if (!placed) {
				this->UnclaimedParentList.Add(next);
				//Create new vertical:
				float angle = (this->UnclaimedParentList.Num() - 1) * this->rotationAmount;
				float radian = UKismetMathLibrary::DegreesToRadians(angle);
				this->newPosition.Y = sin(radian) * this->spaceIncrease;
				this->newPosition.X = cos(radian) * this->spaceIncrease;

				this->UnclaimedConnectionList.Add(this->CreateAndReturnVerticalConnection());
				this->newlyCreatedConnectionIndexes.Add(this->UnclaimedConnectionList.Num() - 1);
			}
		}
		//is this check not the same as hasParent? Can I just use hasParent?
		if (this->UnclaimedParentList.Find(current, this->indexToBeReplaced)) {
			this->SpawnMergeConnection(current->GetActorLocation());

		} else {
			UE_LOG(LogTemp, Warning, TEXT("Could not find commit %s"), *current->GetSha());
		}
	} else {
		this->UpdatePosition(current, next);
	}
	//Always decrease the z direction (down)
	this->newPosition.Z -= this->spaceIncrease;
	this->ScaleVerticalConnections();
	return this->newPosition;
}

//Handles the position update for most parent cases when positioning.
//See RestActor::FindPosition for the only case that does not work with this
void ARestActor::UpdatePosition(ACommitActor* current, ACommitActor* next) {
	int index = 0;
	for (int i = 0; i < this->UnclaimedParentList.Num(); i++) {
		if (this->UnclaimedParentList[i] != NULL && next->GetSha() == this->UnclaimedParentList[i]->GetParentOne()) {
			this->indexesToParentListToRemove.Add(i);
		}
	}

	if (this->indexesToParentListToRemove.Num() > 0) {
		this->indexToBeReplaced = this->indexesToParentListToRemove[0];
		
		float angle = this->indexToBeReplaced * this->rotationAmount;
		float radian = UKismetMathLibrary::DegreesToRadians(angle);
		this->newPosition.Y = sin(radian) * this->spaceIncrease;
		this->newPosition.X = cos(radian) * this->spaceIncrease;
		for (int i = this->indexesToParentListToRemove.Num() - 1; i > 0; i--) {
			//Spawn horizontal branch connectors
			this->SpawnBranchConnection(this->indexesToParentListToRemove[i]);
			this->UnclaimedParentList[this->indexesToParentListToRemove[i]] = NULL;
			this->UnclaimedConnectionList[this->indexesToParentListToRemove[i]] = NULL;
		}
		//Replacing commit in existing track, should not require changes to connections:
		this->UnclaimedParentList[this->indexToBeReplaced] = next;
		//Create certain merge connections for cases where current and next has 2 parents and next has branches from it
		//int index;
		if (this->UnclaimedParentList.Find(current, index) && current->GetParentTwo() == next->GetSha()) {
			this->SpawnMergeConnection(current->GetActorLocation());
		}
	} else {
		bool placed = false;
		//Adding new commit track:
		for (int i = 0; i < this->UnclaimedParentList.Num(); i++) {
			if (this->UnclaimedParentList[i] == NULL) {
				this->UnclaimedParentList[i] = next;
				float angle = i * this->rotationAmount;
				float radian = UKismetMathLibrary::DegreesToRadians(angle);
				this->newPosition.Y = sin(radian) * this->spaceIncrease;
				this->newPosition.X = cos(radian) * this->spaceIncrease;
				this->UnclaimedConnectionList[i] = this->CreateAndReturnVerticalConnection();
				this->newlyCreatedConnectionIndexes.Add(i);
				placed = true;
				index = i;
				break;
			}
		}
		if (!placed) {
			index = this->UnclaimedParentList.Add(next);
			float angle = index * this->rotationAmount;
			float radian = UKismetMathLibrary::DegreesToRadians(angle);
			this->newPosition.Y = sin(radian) * this->spaceIncrease;
			this->newPosition.X = cos(radian) * this->spaceIncrease;
			//Create new vertical:
			this->UnclaimedConnectionList.Add(this->CreateAndReturnVerticalConnection());
			this->newlyCreatedConnectionIndexes.Add(this->UnclaimedConnectionList.Num() - 1);
		}

		//creates connections for case 2 & 2
		if (this->UnclaimedParentList.Find(current, this->indexToBeReplaced)) {
			this->SpawnMergeConnection(current->GetActorLocation());
		} else {
			UE_LOG(LogTemp, Warning, TEXT("Could not find commit %s"), *current->GetSha());
		}
	}
}

AConnectionActor * ARestActor::CreateConnectionActor(FVector conPosition, int zScale, float degreesToRotate) {
	AConnectionActor* conActor = this->GetWorld()->SpawnActor<AConnectionActor>();
	conActor->SetActorLocation(conPosition);
	conActor->setHorizontal();
	this->CheckIfToSetActorHidden(conActor);
	return conActor;
}

AConnectionActor* ARestActor::CreateAndReturnVerticalConnection() {
	AConnectionActor* conActor = this->GetWorld()->SpawnActor<AConnectionActor>();
	FVector position = this->newPosition;
	position.Z -= this->spaceIncrease;
	position.Z += this->spaceIncrease / 2;
	conActor->SetActorLocation(position);
	FRotator rotator = conActor->GetActorRotation();
	rotator.Roll += this->baseRotationForVerticalConnection;
	conActor->SetActorRotation(rotator);
	FVector scale = conActor->GetActorScale();
	scale.Z += this->spaceIncrease ;
	conActor->SetActorScale3D(scale);
	this->CheckIfToSetActorHidden(conActor);
	this->ConnectionArray.Add(conActor);
	return conActor;
}

void ARestActor::ScaleVerticalConnections() {
	int index;
	FVector scale;
	for (int i = 0; i < this->UnclaimedConnectionList.Num(); i++) {
		if (this->UnclaimedConnectionList[i] != NULL && !this->newlyCreatedConnectionIndexes.Find(i,index)){
			scale = this->UnclaimedConnectionList[i]->GetActorScale();
			scale.Z += this->spaceIncrease;
			this->UnclaimedConnectionList[i]->SetActorScale3D(scale);
		}
	}
}

void ARestActor::SpawnBranchConnection(int currentIndex) {
	AConnectionActor* conActor = this->GetWorld()->SpawnActor<AConnectionActor>();
	FVector commitPosition;
	float angle = currentIndex * this->rotationAmount;
	float radian = UKismetMathLibrary::DegreesToRadians(angle);
	commitPosition.Y = sin(radian) * this->spaceIncrease;
	commitPosition.X = cos(radian) * this->spaceIncrease;
	commitPosition.Z = this->newPosition.Z - this->spaceIncrease/2;
	FVector conPosition = this->newPosition;
	conPosition.Z -= this->spaceIncrease;
		this->CheckIfToSetActorHidden(conActor);
	FVector vectorBetween = commitPosition - conPosition;
	float distanceBetween = vectorBetween.Size();
	FRotator rotator = UKismetMathLibrary::MakeRotFromZ(vectorBetween);
	conActor->SetActorLocation(conPosition);
	conActor->SetActorRotation(rotator);
	conActor->SetActorScale3D(FVector(1, 1, distanceBetween));
	this->CheckIfToSetActorHidden(conActor);
	this->ConnectionArray.Add(conActor);
}

void ARestActor::SpawnMergeConnection(FVector currentPosition) {
	AConnectionActor* conActor = this->GetWorld()->SpawnActor<AConnectionActor>();
	FVector conPosition = this->newPosition;
	conPosition.Z -= this->spaceIncrease / 2;
		this->CheckIfToSetActorHidden(conActor);
	FVector vectorBetween = currentPosition - conPosition;
	float distanceBetween = vectorBetween.Size();
	FRotator rotator = UKismetMathLibrary::MakeRotFromZ(vectorBetween);
	conActor->SetActorLocation(conPosition);
	conActor->SetActorRotation(rotator);
	conActor->SetActorScale3D(FVector(1, 1, distanceBetween));
	this->CheckIfToSetActorHidden(conActor);
	this->ConnectionArray.Add(conActor);
}

void ARestActor::SetFloorActorReference(AStaticMeshActor* floorMesh) {
	this->floor = floorMesh;
	this->SetCompareValueForFloor();
}

void ARestActor::SetCompareValueForFloor() {
	this->modifiedHeightFromFloor = this->floor->GetActorLocation().Z - this->spaceIncrease;
}

void ARestActor::CheckIfToSetActorHidden(AActor * actorToBeHidden) {
	bool hidden;
	if (actorToBeHidden->GetActorLocation().Z < modifiedHeightFromFloor) {
		hidden = true;
	} else {
		hidden = false;
	}
	actorToBeHidden->SetActorHiddenInGame(hidden);
}