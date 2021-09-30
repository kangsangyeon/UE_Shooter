// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"

#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() :
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffset(0.f),
	LastMovementOffset(0.f),
	bAiming(false),
	CharacterYaw(0.f),
	CharacterYawLastFrame(0.f),
	RootYawOffset(0.f)
{
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());

	if (ShooterCharacter == nullptr)
		return;

	// 캐릭터의 velocity로부터 속도를 얻습니다.
	FVector Velocity{ShooterCharacter->GetVelocity()};
	Velocity.Z = 0;
	Speed = Velocity.Size();

	// 캐릭터가 공중에 있는지 확인합니다.
	bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

	// 캐릭터가 가속중인지 확인합니다.
	bIsAccelerating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0;

	// 에임 회전값과 이동 방향 회전값의 차이(Offset)를 얻습니다.
	const FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
	const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Velocity);
	MovementOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	// 움직일 때에만 Last Movement Offset값을 새로고칩니다.
	if (ShooterCharacter->GetVelocity().Size() > 0)
		LastMovementOffset = MovementOffset;

	// 조준중인지에 대한 여부를 얻습니다.
	bAiming = ShooterCharacter->GetAiming();

	// 제자리 돌기 관련 변수를 새로고칩니다.
	TurnInPlace();

	// Debug
	const FString AimRotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
	const FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);
	const FString RotationOffsetMessage = FString::Printf(TEXT("Movement Offset: %f"), MovementOffset);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 0, FColor::White, AimRotationMessage);
		GEngine->AddOnScreenDebugMessage(2, 0, FColor::White, MovementRotationMessage);
		GEngine->AddOnScreenDebugMessage(3, 0, FColor::White, RotationOffsetMessage);
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr)
		return;

	// 캐릭터는 움직일 때 제자리 돌기를 하지 않습니다.
	if (Speed > 0)
		return;

	CharacterYawLastFrame = CharacterYaw;
	CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;

	// 직전 프레임에 비해 이번 프레임에서 어느정도 회전했는지에 대한 차이를 얻고,
	// 그 차이를 RootYawOffset에 누적합니다.
	const float YawDelta{CharacterYaw - CharacterYawLastFrame};
	RootYawOffset -= YawDelta;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(20, 0.f, FColor::White, FString::Printf(TEXT("CharacterYaw %f"), CharacterYaw));
		GEngine->AddOnScreenDebugMessage(21, 0.f, FColor::White, FString::Printf(TEXT("RootYawOffset %f"), RootYawOffset));
	}
}
