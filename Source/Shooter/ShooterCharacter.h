// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AmmoType.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	ECS_Max UMETA(DisplayName = "DefaultMax")
};

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
	 * @brief 총구에서부터 Crosshair 너머 바라보는 방향으로 Linecast를 실시하여 충돌한 위치값을 얻어냅니다.
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

	void OnFireButtonPressed();
	void OnFireButtonReleased();

	void StartAutoFireTimer();

	UFUNCTION()
	void OnEndAutoFireTimer();

	/**
	 * @brief Crosshair 너머로 LineTrace를 실시한 결과를 얻습니다.
	 * @param OutHitResult LineTrace를 실시한 결과입니다.
	 * @param OutHitLocation LineTrace를 실시하여 충돌한 지점입니다. 충돌하지 않았을 경우 BeamEndPoint(바라보는 허공의 지점)이 반환됩니다.
	 * @return LineTrace를 실시한 결과 충돌한 무언가가 있는지에 대한 여부를 반환합니다.
	 */
	const bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	/**
	 * @brief Tick에서 OverlappedItemCount > 0일 때에만 호출되어 Item Trace를 실시합니다.
	 */
	void TraceForItems();

	/**
	 * @brief 기본 무기를 스폰합니다.
	 */
	class AWeapon* SpawnDefaultWeapon() const;

	/**
	 * @brief 무기를 장착합니다.
	 */
	void EquipWeapon(class AWeapon* WeaponToEquip);

	/**
	 * @brief 장착중인 무기를 떼어내고 땅에 떨어뜨립니다.
	 */
	void DropWeapon();

	void InteractButtonPressed();
	void InteractButtonReleased();

	/**
	 * @brief 현재 장착중인 무기를 버리고 TraceHitItem을 새로 장착합니다.
	 */
	void SwapWeapon(AWeapon* WeaponToSwap);

	/**
	 * @brief 기본 Ammo값으로 AmmoMap을 초기화합니다.
	 */
	void InitializeAmmoMap();

	/**
	 * @brief 장착한 무기가 탄약을 가지고 있는지 확인합니다.
	 */
	bool WeaponHasAmmo();

	void PlayFireSound();

	void SendBullet();

	void PlayGunFireMontage();

	void OnReloadButtonPressed();

	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	void OnFinishedReloading();

	bool CarryingAmmo();

	/**
	 * @brief 애니메이션 블루프린트에서 OnGrabClip notify가 발생할 때 호출될 메소드입니다.
	 * OnGrabClip notify는 재장전 애니메이션에서 탄창을 잡기 시작할 때 발생하는 애님 노티파이입니다.
	 */
	UFUNCTION(BlueprintCallable)
	void OnGrabClip();

	/**
	* @brief 애니메이션 블루프린트에서 OnReleaseClip notify가 발생할 때 호출될 메소드입니다.
	* OnReleaseClip notify는 재장전 애니메이션에서 재장전을 마치고 탄창을 다시 총에 장착할 때 발생하는 애님 노티파이입니다.
	 */
	UFUNCTION(BlueprintCallable)
	void OnReleaseClip();
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetPickupItem(class AItem* Item);

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

	/**
	 * @brief 총알 발사 버튼을 누르고 있는중인지에 대한 여부입니다.
	 */
	bool bFireButtonPressed;

	/**
	 * @brief 총알 발사 속도입니다.
	 */
	float AutoFireRate;

	/**
	 * @brief 총알 발사간의 타이머입니다.
	 */
	FTimerHandle AutoFireTimer;

	/**
	 * @brief 곂쳐져있는 아이템의 개수입니다.
	 * 곂쳐있는 아이템의 개수가 0이 아닐 때에만 매 프레임 아이템 Trace를 실시합니다.
	 */
	int8 OverlappedItemCount;

	/**
	 * @brief 직전 프레임에 LineTrace를 실시하여 검출된 아이템입니다.
	 * 지난 프레임에서 아이템을 바라보고 있지 않았다면 nullptr입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess="true"))
	class AItem* TraceHitItemLastFrame;

	/**
	* @brief 이번 프레임에 LineTrace를 실시하여 검출된 아이템입니다.
	* 아이템을 바라보고 있지 않을 때 nullptr입니다.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items", meta=(AllowPrivateAccess="true"))
	AItem* TraceHitItem;

	/**
	 * @brief Item Interp가 진행될 때 카메라로부터 얼만큼 떨어져있는 곳으로 이동되기를 원하는지에 대한 값입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items", meta=(AllowPrivateAccess="true"))
	FVector ItemInterpDesiredOffset;

	/**
	* @brief 장착하고 있는 무기입니다.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess="true"))
	class AWeapon* EquippedWeapon;

	/**
	* @brief 게임이 시작될 때 기본적으로 장착될 무기의 클래스입니다.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess="true"))
	TSubclassOf<class AWeapon> DefaultWeaponClass;

	/**
	 * @brief 탄약의 종류와 그 소지 개수를 저장하는 Map입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess="true"))
	TMap<EAmmoType, int32> AmmoMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess="true"))
	int32 Starting9mmAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess="true"))
	int32 StartingARAmmo;

	/**
	 * @brief 캐릭터의 전투 상태입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess="true"))
	ECombatState CombatState;

	/**
	 * @brief 재장전 애니메이션에 사용될 Montage 애셋입니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess="true"))
	UAnimMontage* ReloadMontage;

	/**
	 * @brief 재장전 애니메이션을 재생할 때, 맨 처음에 Clip을 잡았을 때의 Clip의 Transform입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess="true"))
	FTransform ClipTransform;

	/**
	 * @brief 재장전 애니메이션을 재생할 때, 캐릭터의 손에 부착할 Scene Component입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess="true"))
	USceneComponent* LeftHandSceneComponent;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; };

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; };

	FORCEINLINE bool GetAiming() const { return bAiming; };

	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	FORCEINLINE void IncreaseOverlappedItemCount(int8 Amount) { OverlappedItemCount = FMath::Clamp(OverlappedItemCount + Amount, 0, INT_MAX); }

	FORCEINLINE bool GetShouldForItemTrace() const { return OverlappedItemCount > 0; }

	FVector GetItemInterpDesiredDestination() const;
};
