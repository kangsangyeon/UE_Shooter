// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AmmoType.h"
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
	 * @brief 한 탄창에 들어가는 탄약의 개수입니다.
	 */
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess="true"))
	int32 MagazineCapacity;

	/**
	 * @brief 무기의 종류입니다.
	 */
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess="true"))
	EWeaponType WeaponType;

	/**
	 * @brief 무기가 사용하는 탄약의 종류입니다.
	 */
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess="true"))
	EAmmoType AmmoType;

	/**
	 * @brief 재장전시 재생될 애니메이션 Montage의 Section 이름입니다.
	 */
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess="true"))
	FName ReloadMontageSectionName;

	/**
	 * @brief 무기 SkeletalMesh의 Clip에 해당되는 Bone의 이름입니다.
	 */
	UPROPERTY(Editanywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess="true"))
	FName ClipBoneName;

	/**
	* @brief 이 무기를 장착한 캐릭터가 재장전 애니메이션을 재생하고 있으며 clip을 움직이고 있는지에 대한 여부입니다.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess="true"))
	bool bMovingClip;

public:
	FORCEINLINE int32 GetAmmoCount() const { return AmmoCount; }

	void DecrementAmmoCount();

	void ReloadAmmo(int32 Amount);

	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

	FORCEINLINE FName GetReloadMontageSectionName() const { return ReloadMontageSectionName; }

	FORCEINLINE int32 GetClipBoneIndex() const { return Mesh->GetBoneIndex(ClipBoneName); }

	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }
};
