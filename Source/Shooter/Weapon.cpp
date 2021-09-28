// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() :
	ThrowWeaponTime(.7f),
	bFalling(false),
	AmmoCount(0),
	WeaponType(EWeaponType::EWT_SubmachineGun)
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
