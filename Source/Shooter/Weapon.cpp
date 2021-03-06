// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() :
	ThrowWeaponTime(.7f),
	bFalling(false),
	AmmoCount(0),
	MagazineCapacity(30),
	WeaponType(EWeaponType::EWT_SubmachineGun),
	AmmoType(EAmmoType::EAT_9mm),
	ClipBoneName(TEXT("smg_clip")),
	bMovingClip(false)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 아이템이 떨어질 때, 이상하게 기울어지지 않도록 합니다.
	if (ItemState == EItemState::EIS_Falling && bFalling)
	{
		const FRotator MeshRotation{0.f, Mesh->GetComponentRotation().Yaw, 0.f};
		Mesh->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::BeDropped()
{
	Super::BeDropped();

	BeThrown();
}

void AWeapon::BeThrown()
{
	const FRotator MeshRotation{0.f, Mesh->GetComponentRotation().Yaw, 0.f};
	Mesh->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{Mesh->GetForwardVector()};
	const FVector MeshRight{Mesh->GetRightVector()};

	// 무기를 던질 방향을 구합니다.
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	float RandomRotation = FMath::FRandRange(0.f, 30.f);
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector{0.f, 0.f, 1.f});

	// 무기를 던집니다.
	const FVector ImpulseForce = ImpulseDirection * 2'000;
	Mesh->AddImpulse(ImpulseForce);

	// ThrowWeaponTimer를 시작합니다.
	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}

void AWeapon::DecrementAmmoCount()
{
	if (AmmoCount - 1 < 0)
		AmmoCount = 0;
	else
		--AmmoCount;
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(AmmoCount + Amount <= MagazineCapacity, TEXT("탄창의 크기보다 더 많은 양의 탄약을 장전하려 합니다."));
	AmmoCount += Amount;
}
