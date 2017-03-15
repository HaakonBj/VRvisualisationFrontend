// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ConnectionActor.generated.h"

UCLASS()
class VRVISFRONTEND_API AConnectionActor : public AActor {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	USceneComponent* rootSceneComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	USceneComponent* sceneComponentForMergeComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	UStaticMeshComponent* cylinderVisual;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	UStaticMeshComponent* cylinderVisualMergeAddition;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rest")
	bool horizontal;

	AConnectionActor();
	UFUNCTION(BlueprintCallable, Category = "Rest")
	void Init(bool verticalType);
	void setHorizontal();
	void SetMergeConnection();

	template <typename ObjClass>
	static FORCEINLINE ObjClass* LoadObjFromPath(const FName& Path) {
		if (Path == NAME_None) return NULL;
		return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), NULL, *Path.ToString()));
	}
	static FORCEINLINE UStaticMesh* LoadMeshFromPath(const FName& Path) {
		if (Path == NAME_None) return NULL;
		return LoadObjFromPath<UStaticMesh>(Path);
	}
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
};
