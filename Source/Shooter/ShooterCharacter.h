// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*
	 * 전진/후진 입력에 의해 호출됩니다.
	 */
	void MoveForward(float Value);

	/*
	 * 옆 이동에 의해 호출됩니다.
	 */
	void MoveRight(float Value);

	/**
	 * @brief 주어진 Rate으로 좌우 회전 입력을 할 때 호출됩니다.
	 * @param Rate 이 값은 normalized된 값입니다. (-1 ~ 1)
	 * 1.0은 TurnRate를 100% 반영하는 것을 의미합니다.
	 */
	void TurnAtRate(float Rate);

	/**
	 * @brief 주어진 Rate으로 위아래 회전 입력을 할 때 호출됩니다.
	 * @param Rate 이 값은 normalized된 값입니다. (-1 ~ 1)
	 * 1.0은 TurnRate를 100% 반영하는 것을 의미합니다.
	 */
	void LookUpAtRate(float Rate);

	/**
	 * @brief 마우스로 좌우 회전 입력을 할 때 호출됩니다.
	 * @param Value 마우스 좌우 회전 입력값입니다.
	 */
	void Turn(float Value);

	/**
	 * @brief 마우스로 위아래 회전 입력을 할 때 호출됩니다.
	 * @param Value 마우스 위아래 회전 입력값입니다.
	 */
	void LookUp(float Value);

	/**
	 * @brief FireButton이 눌렸을 때 호출됩니다.
	 */
	void FireWeapon();

	/**
	 * @brief Crosshair 너머 바라보는 방향으로 Linecast를 실시하여 충돌한 위치값을 얻어냅니다.
	 * 만약 충돌한 오브젝트가 없다면 바라보는 방향으로 먼 거리의 지점이 반환됩니다.
	 */
	const bool GetBeamEndPoint(const FVector& MuzzleSocketLocation, FVector& OutBeamEndPoint);

	/**
	 * @brief Aiming 버튼을 눌렀는지 떼었는지에 따라 bAiming값을 true 또는 false로 설정합니다.
	 */
	void AimingButtonPressed();
	void AimingButtonReleased();

	void CameraInterpolation(float DeltaTime);

	/**
	 * @brief 조준하고 있는지에 대한 여부에 따라 BaseTurnRate와 BaseLookUpRate를 설정합니다.
	 */
	void SetLookRates();

	void CalculateCrosshairSpread(float DeltaTime);
	
	void StartCrosshairFireTimer();

	UFUNCTION()
	void OnFinishedCrosshairFireTimer();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/*
	 * CameraBoom은 캐릭터 뒤에서 카메라의 위치를 조정하는 컴포넌트입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess="true"))
	class USpringArmComponent* CameraBoom;

	/*
	 * 캐릭터를 따라가는 카메라 컴포넌트입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/*
	 * 기본 좌우 회전 속도입니다. (deg/sec)
	 * 다른 요소들이 최종 회전 속도에 영향을 미칠 수 있습니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/*
	 * 기본 위아래 회전 속도입니다. (deg/sec)
	 * 다른 요소들이 최종 회전 속도에 영향을 미칠 수 있습니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/**
	 * @brief 조준중이지 않을 때 사용될 좌우 회전 속도입니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	/**
	 * @brief 조준중이지 않을 때 사용될 위아래 회전 속도입니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/**
	 * @brief 조준중일 때 사용될 좌우 회전 속도입니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	/**
	 * @brief 조준중일 때 사용될 위아래 회전 속도입니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/**
	 * @brief 마우스로 조작하며 조준중이지 않을 때 사용될 좌우 회전 민감도 Scale Factor입니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float MouseHipTurnRate;

	/**
	 * @brief 마우스로 조작하며 조준중이지 않을 때 사용될 위아래 회전 민감도 Scale Factor입니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	/**
	 * @brief 마우스로 조작하며 조준중일 때 사용될 좌우 회전 민감도 Scale Factor입니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	/**
	 * @brief 마우스로 조작하며 조준중일 때 사용될 위아래 회전 민감도 Scale Factor입니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	/**
	 * @brief 랜덤한 총소리를 내는 사운드 큐입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	/**
	 * @brief BarrelSocket에 생성될 불꽃 파티클입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	/**
	 * @brief 총 발사 AnimationMontage입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	/**
	 * @brief 총알 충격에 생성될 파티클입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;

	/**
	 * @brief 총알 궤적 표현에 사용될 파티클입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* BeamParticles;

	/**
	 * @brief Aiming중인지에 대한 여부입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/**
	 * @brief 기본 카메라 Field of View 값입니다.
	 */
	float CameraDefaultFOV;

	/**
	 * @brief 확대 상태의 카메라 Field of View값입니다.
	 */
	float CameraZoomFOV;

	/**
	 * @brief 현재 프레임의 카메라 FOV 값입니다.
	 */
	float CameraCurrentFOV;

	/**
	 * @brief 조준 또는 해제시에 변화할 FOV값의 보간 속도입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	/**
	 * @brief Crosshair가 벌어지는 정도를 결정합니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	/**
	 * @brief Velocity 컴포넌트용 Crosshair Spread입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	/**
	 * @brief Air 컴포넌트용 Crosshair Spread입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	/**
	 * @brief Aim 컴포넌트용 Crosshair Spread입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	/**
	 * @brief Firing 컴포넌트용 Crosshair Spread입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairFiringFactor;

	float FireTimerDuration;

	bool bFiring;

	FTimerHandle CrosshairFireTimer;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; };

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; };

	FORCEINLINE bool GetAiming() const { return bAiming; };

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;
};
