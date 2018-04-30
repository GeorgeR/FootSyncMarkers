#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"

#include "FootSyncMarkersModifier.generated.h"

UENUM(BlueprintType)
enum class ERootMotionDirection : uint8
{
	RMD_PosX = 0		UMETA(DisplayName = "+X"),
	RMD_PosY = 1		UMETA(DisplayName = "+Y"),
	RMD_PosZ = 2		UMETA(DisplayName = "+Z"),
	RMD_NegX = 3		UMETA(DisplayName = "-X"),
	RMD_NegY = 4		UMETA(DisplayName = "-Y"),
	RMD_NegZ = 5		UMETA(DisplayName = "-Z")
};

USTRUCT(BlueprintType)
struct FOOTSYNCMARKERS_API FModifiedBone
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName BoneName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Offset;
};

class UAnimSequence;

UCLASS(BlueprintType)
class FOOTSYNCMARKERS_API UFootSyncMarkersModifier 
	: public UAnimationModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString PathFilter;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ERootMotionDirection RootMotionDirection;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bCreateCurve;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName PelvisBoneName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FModifiedBone> FootBones;

	UFootSyncMarkersModifier();

	void OnApply_Implementation(UAnimSequence* InAnimationSequence) override;
	void OnRevert_Implementation(UAnimSequence* InAnimationSequence) override;

private:
	UPROPERTY()
	FName SyncTrackName;

	FVector GetBoneLocationRelativeToTime(UAnimSequence* InAnimationSequence, FName InBoneName, FName InPelvisBoneName, float InTime);

	inline bool ShouldApply(UAnimSequence* InAnimationSequence);
	inline void RemoveAll(UAnimSequence* InAnimationSequence);
	inline float SetTime(UAnimSequence* InAnimationSequence, int32 InFrameIndex);
	inline float GetZeroTime(UAnimSequence* InAnimationSequence, float InBoneLocation, float InLastBoneLocation, float InTime);
};