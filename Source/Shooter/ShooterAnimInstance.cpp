// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"

#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}
