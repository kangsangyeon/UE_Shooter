// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"

#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GeometryCollection/GeometryCollectionUtility.h"
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
	RootYawOffset(0.f),
	AimingPitch(0.f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip)
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

	// 재장전중인지에 대한 여부를 얻습니다.
	bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;

	// 제자리 돌기 관련 변수를 새로고칩니다.
	TurnInPlace();

	// 현재 캐릭터의 행동에 알맞은 Offset State를 가지도록 새로고칩니다.
	UpdateOffsetState();

	// Debug
	const FString AimRotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
	const FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);
	const FString RotationOffsetMessage = FString::Printf(TEXT("Movement Offset: %f"), MovementOffset);
	const FString OffsetStateMessage = FString::Printf(TEXT("OffsetState %hhd"), OffsetState);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 0, FColor::White, AimRotationMessage);
		GEngine->AddOnScreenDebugMessage(2, 0, FColor::White, MovementRotationMessage);
		GEngine->AddOnScreenDebugMessage(3, 0, FColor::White, RotationOffsetMessage);
		GEngine->AddOnScreenDebugMessage(4, 0.f, FColor::White, OffsetStateMessage);
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateOffsetState()
{
	// 위에 있는 조건일 수록 우선순위가 높다는 것에 유의하세요.
	if (bReloading)
		OffsetState = EOffsetState::EOS_Reloading;
	else if (bIsInAir)
		OffsetState = EOffsetState::EOS_InAir;
	else if (bAiming)
		OffsetState = EOffsetState::EOS_Aiming;
	else
		OffsetState = EOffsetState::EOS_Hip;
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr)
		return;

	CharacterYawLastFrame = CharacterYaw;
	CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;

	// 캐릭터가 바라보는 방향 Rotation으로부터 Pitch를 얻습니다.
	AimingPitch = ShooterCharacter->GetBaseAimRotation().Pitch;

	if (Speed > 0 || bIsInAir)
	{
		/* 캐릭터가 움직이거나 공중에 있을 때에는
		 * 플레이어가 바라보는 곳을 바라보도록 방향을 바로 바꿉니다. */

		// 캐릭터가 정면을 바라보도록 합니다.
		RootYawOffset = 0;

		// 제자리 돌기 관련 변수들을 초기화합니다.
		RotationCurveLastFrame = 0;
		RotationCurve = 0;
	}
	else
	{
		/* 캐릭터는 방향키로 움직이지 않을 때에만 제자리 돌기를 합니다. */

		// 직전 프레임에 비해 이번 프레임에서 어느정도 회전했는지에 대한 차이를 얻고,
		// 그 차이를 RootYawOffset에 누적합니다.
		const float YawDelta{CharacterYaw - CharacterYawLastFrame};
		// RootYawOffset은 회전값이기 때문에 [-180, 180] 사이의 값으로 Clamp합니다.
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);


		// 제자리 돌기 애니메이션이 재생중일 때에만 처리합니다.
		const float Turning{GetCurveValue(TEXT("Turning"))};
		if (Turning > 0)
		{
			// 지난 프레임과 이번 프레임의 RotationCurve값의 차이를 구합니다.
			// 이번 프레임에서 캐릭터는 이 차이값만큼 회전합니다.
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{RotationCurve - RotationCurveLastFrame};

			// RootYawOffset > 0 이라면 왼쪽으로 돌고있다는 것을 의미하고,
			// RootYawOffset < 0 이라면 오른쪽으로 돌고있다는 것을 의미합니다.
			// 어느 쪽으로 돌고있는지에 따라 RootYawOffset에 값을 빼거나 더합니다.
			// 
			// 예를 들어, RootYawOffset이 90을 넘기 시작하여 왼쪽으로 돌기 시작했다면, 왼쪽으로 제자리 돌기 애니메이션이 재생될 것이고
			// 애니메이션을 재생하면서 실제로 왼쪽으로 회전해야 하기 때문에 RootYawRotation 값을 점차 0이 되도록 DeltaRotation을 매 프레임마다 빼주어야 합니다.
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			// 빠르게 회전하여 90도보다 더 큰 값으로 회전했다면,
			// 90도를 초과한 만큼의 차이는 바로 없앱니다.
			// 이렇게 함으로써 캐릭터는 카메라가 바라보는 방향과 근사한 방향으로 바라볼 수 있게 됩니다.
			const float AbsRootYawOffset{FMath::Abs(RootYawOffset)};
			if (AbsRootYawOffset > 90)
			{
				const float YawExcess{AbsRootYawOffset - 90.f};
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}

		// Debug
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(20, 0.f, FColor::White, FString::Printf(TEXT("CharacterYaw %f"), CharacterYaw));
			GEngine->AddOnScreenDebugMessage(21, 0.f, FColor::White, FString::Printf(TEXT("RootYawOffset %f"), RootYawOffset));
		}
	}
}
