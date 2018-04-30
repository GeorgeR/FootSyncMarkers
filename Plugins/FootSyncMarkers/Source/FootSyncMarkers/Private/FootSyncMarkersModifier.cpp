#include "FootSyncMarkersModifier.h"
#include "Animation/AnimSequence.h"
#include "AnimationBlueprintLibrary.h"
#include "AnimCurveTypes.h"
#include "Kismet/KismetMathLibrary.h"

UFootSyncMarkersModifier::UFootSyncMarkersModifier()
{
	PathFilter = TEXT("");
	RootMotionDirection = ERootMotionDirection::RMD_PosY;
	bCreateCurve = false;
	PelvisBoneName = TEXT("Pelvis");

	FModifiedBone LeftFoot;
	LeftFoot.BoneName = TEXT("foot_l");
	FootBones.Add(LeftFoot);

	FModifiedBone RightFoot;
	RightFoot.BoneName = TEXT("foot_r");
	FootBones.Add(RightFoot);

	SyncTrackName = TEXT("Sync");
}

void UFootSyncMarkersModifier::OnApply_Implementation(UAnimSequence* InAnimationSequence)
{
	if (!ShouldApply(InAnimationSequence))
		return;

	RemoveAll(InAnimationSequence);

	UAnimationBlueprintLibrary::AddAnimationNotifyTrack(InAnimationSequence, SyncTrackName);
	auto RootMotionDirectionI = StaticCast<uint8>(RootMotionDirection);

	for(auto& Foot : FootBones)
	{
		if(bCreateCurve)
			UAnimationBlueprintLibrary::AddCurve(InAnimationSequence, Foot.BoneName, ERawCurveTrackTypes::RCT_Float, false);

		auto LastBoneLocation = 0.0f;

		int32 NumFrames = 0;
		UAnimationBlueprintLibrary::GetNumFrames(InAnimationSequence, NumFrames);

		for(auto i = 0; i < NumFrames; i++)
		{
			auto Time = SetTime(InAnimationSequence, i);
			auto Location = GetBoneLocationRelativeToTime(InAnimationSequence, Foot.BoneName, PelvisBoneName, Time);
			auto BoneLocation = RootMotionDirectionI <= 3 ? Location.Component(RootMotionDirectionI) : (Location * -1).Component(RootMotionDirectionI - 3);
			BoneLocation += Foot.Offset;

			if (bCreateCurve)
				UAnimationBlueprintLibrary::AddFloatCurveKey(InAnimationSequence, Foot.BoneName, Time, BoneLocation);

			if(i != 0 && (FMath::Sign(LastBoneLocation) != FMath::Sign(BoneLocation)))
			{
				auto SyncMarkerName = FName(*Foot.BoneName.ToString().Append(BoneLocation >= 0.0f ? TEXT("_step_forward") : TEXT("_step_backward")));
				UAnimationBlueprintLibrary::AddAnimationSyncMarker(InAnimationSequence, SyncMarkerName, GetZeroTime(InAnimationSequence, BoneLocation, LastBoneLocation, Time), SyncTrackName);
			}

			LastBoneLocation = BoneLocation;

			UAnimationBlueprintLibrary::FinalizeBoneAnimation(InAnimationSequence);
		}
	}
}

void UFootSyncMarkersModifier::OnRevert_Implementation(UAnimSequence* InAnimationSequence)
{
	if (!ShouldApply(InAnimationSequence))
		return;

	RemoveAll(InAnimationSequence);
}

FVector UFootSyncMarkersModifier::GetBoneLocationRelativeToTime(UAnimSequence* InAnimationSequence, FName InBoneName, FName InPelvisBoneName, float InTime)
{
	TArray<FName> BonePath; 
	UAnimationBlueprintLibrary::FindBonePathToRoot(InAnimationSequence, InBoneName, BonePath);

	auto Transform = FTransform::Identity;
	for(auto& Bone : BonePath)
	{
		if (Bone == InBoneName)
			break;

		FTransform Pose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(InAnimationSequence, Bone, InTime, false, Pose);
		Transform = UKismetMathLibrary::ComposeTransforms(Transform, Pose);
	}

	return Transform.GetLocation();
}

bool UFootSyncMarkersModifier::ShouldApply(UAnimSequence* InAnimationSequence)
{
	return PathFilter.IsEmpty() || InAnimationSequence->GetPathName().Contains(PathFilter);
}

void UFootSyncMarkersModifier::RemoveAll(UAnimSequence* InAnimationSequence)
{
	UAnimationBlueprintLibrary::RemoveAnimationNotifyTrack(InAnimationSequence, SyncTrackName);
	for(auto& Foot : FootBones)
	{
		if (!UAnimationBlueprintLibrary::DoesCurveExist(InAnimationSequence, Foot.BoneName, ERawCurveTrackTypes::RCT_Float))
			continue;

		UAnimationBlueprintLibrary::RemoveCurve(InAnimationSequence, Foot.BoneName);
	}
}

float UFootSyncMarkersModifier::SetTime(UAnimSequence* InAnimationSequence, int32 InFrameIndex)
{
	auto Time = 0.0f; 
	UAnimationBlueprintLibrary::GetTimeAtFrame(InAnimationSequence, InFrameIndex, Time);

	auto FrameCount = 0; 
	UAnimationBlueprintLibrary::GetNumFrames(InAnimationSequence, FrameCount);

	Time = InFrameIndex == FrameCount ? 0.001f : Time;

	return Time;
}

float UFootSyncMarkersModifier::GetZeroTime(UAnimSequence* InAnimationSequence, float InBoneLocation, float InLastBoneLocation, float InTime)
{
	auto SequenceLength = 0.0f;
	UAnimationBlueprintLibrary::GetSequenceLength(InAnimationSequence, SequenceLength);

	auto NumFrames = 0;
	UAnimationBlueprintLibrary::GetNumFrames(InAnimationSequence, NumFrames);

	return InBoneLocation == 0.0f ? InTime : InTime - SequenceLength / StaticCast<float>(NumFrames) * FMath::Abs(InBoneLocation / (InBoneLocation - InLastBoneLocation));
}