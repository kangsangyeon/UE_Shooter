// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),

	EWT_Max UMETA(DisplayName = "DefaultMax")
};

/**
 * 
 */
UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;

	virtual void BeDropped() override;

protected:
	void BeThrown();

	void StopFalling();

private:
	FTimerHandle ThrowWeaponTimer;

	float ThrowWeaponTime;

	bool bFalling;

	/**
	 * @brief 무기에 포함되어 있는 탄약 개수입니다.
	 */
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess="true"))
	int32 AmmoCount;

	/**
	 * @brief 무기의 종류입니다.
	 */
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess="true"))
	EWeaponType WeaponType;

public:
	FORCEINLINE int32 GetAmmoCount() const { return AmmoCount; }

	void DecrementAmmoCount();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
};
