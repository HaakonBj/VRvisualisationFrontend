#pragma once
#include "FArr.generated.h"

USTRUCT()
struct FArr {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Array Struct")
	TArray<FString> arr;
};