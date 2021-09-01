// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"

#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
	MovementOffset = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, MovementRotation).Yaw;

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
