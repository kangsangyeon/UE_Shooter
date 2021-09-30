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
	UShooterAnimInstance();
	
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;

protected:
	/**
	* @brief 제자리 돌기(Turn in Place) 구현을 위한 변수들을 갱신합니다.
	*/
	void TurnInPlace();

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

	/**
	* @brief 이번 프레임에서의 캐릭터 Yaw입니다.
	*/
	float CharacterYaw;

	/**
	* @brief 직전 프레임에서의 캐릭터 Yaw입니다.
	*/
	float CharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation - Turn in Place", meta=(AllowPrivateAccess="true"))
	float RootYawOffset;

	/**
	 * @brief 이번 프레임의 제자리 돌기 애니메이션에 포함된 Rotation Curve 값입니다.
	 * 만약 이번 프레임에 제자리 돌기 애니메이션이 재생중이지 않았다면 이 값은 갱신되지 않습니다.
	 */
	float RotationCurve;

	/**
	* @brief 직전 프레임의 제자리 돌기 애니메이션에 포함된 Rotation Curve 값입니다.
	* 만약 직전 프레임에서 제자리 돌기 애니메이션이 재생중이지 않았다면 이 값은 갱신되지 않습니다.
	 */
	float RotationCurveLastFrame;
};
