// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement, meta=(AllowPrivateAccess="true"))
	class AShooterCharacter* ShooterCharacter;

	/**
	 * @brief 캐릭터의 속도입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	/**
	 * @brief 캐릭터가 공중에 있는지 없는지에 대한 여부입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	/**
	 * @brief 캐릭터가 이동중인지 아닌지에 대한 여부입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/**
	 * @brief Strafing에 사용되는 OffsetYaw입니다.
	 * OffsetYaw값은 에임 회전값과 이동 방향 회전값의 Yaw 차이(Offset)입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta= (AllowPrivateAccess = "true"))
	float MovementOffset;

	/**
	 * @brief 가장 마지막으로 움직였을 때의 MovementOffset입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffset;

	/**
	 * @brief Aiming중인지에 대한 여부입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;
};
