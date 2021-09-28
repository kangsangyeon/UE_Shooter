// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "Item.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	// 기본 회전 속도
	BaseTurnRate(45.0f),
	BaseLookUpRate(45.0f),
	// Aiming/Not Aiming 회전 속도
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	// 마우스를 사용하여 회전할 때 사용될 민감도 Scale Factor
	MouseHipTurnRate(1.f),
	MouseHipLookUpRate(1.f),
	MouseAimingTurnRate(0.5f),
	MouseAimingLookUpRate(0.5f),

	bAiming(false),
	// 카메라 FOV와 보간 속도
	CameraDefaultFOV(0.f),
	CameraCurrentFOV(0.f),
	CameraZoomFOV(35.f),
	ZoomInterpSpeed(20.f),
	// Crosshair Spread Factor
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairFiringFactor(0.f),
	// 총알 발사 타이머 변수
	FireTimerDuration(.05),
	bFiring(false),
	// 자동 발사 타이머 변수
	AutoFireRate(0.1f),
	// 아이템 Trace
	OverlappedItemCount(0),
	// 아이템 Interp
	ItemInterpDesiredOffset(FVector{250.f, 0.f, 60.f}),
	// 기본 탄약 개수
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	// Combat 변수
	CombatState(ECombatState::ECS_Unoccupied)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// CameraBoom 생성 (
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f;
	CameraBoom->bUsePawnControlRotation = true; // Controller에 따라 Arm을 회전시킵니다.
	CameraBoom->SocketOffset = FVector{0.f, 50.f, 70.f}; // 카메라가 캐릭터의 살짝 옆에서 비추도록 offset을 설정합니다.

	// FollowCamera 생성
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Controller가 회전할 때 Pawn도 같이 회전하도록 설정합니다.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// CharacterMovement 설정
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator{0.f, 540.f, 0.f};

	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}


// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	// 기본 무기를 스폰하고 장착합니다.
	EquipWeapon(SpawnDefaultWeapon());

	// 기본 탄약을 얻습니다.
	InitializeAmmoMap();
}

void AShooterCharacter::MoveForward(float Value)
{
	if (Controller == nullptr || Value == 0)
		return;

	const FRotator Rotation{Controller->GetControlRotation()};
	const FRotator YawRotation{0, Rotation.Yaw, 0};

	const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
	AddMovementInput(Direction, Value);
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller == nullptr || Value == 0)
		return;

	const FRotator Rotation{Controller->GetControlRotation()};
	const FRotator YawRotation{0, Rotation.Yaw, 0};

	const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
	AddMovementInput(Direction, Value);
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	const float TurnScaleFactor{bAiming ? MouseAimingTurnRate : MouseHipTurnRate};

	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	const float LookUpScaleFactor{bAiming ? MouseAimingLookUpRate : MouseHipLookUpRate};

	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied)
		return;

	if (WeaponHasAmmo() == false)
		return;

	EquippedWeapon->DecrementAmmoCount();

	// 발사 효과음을 재생합니다.
	PlayFireSound();

	// 총알을 발사합니다.
	SendBullet();

	// 발사 애니메이션을 재생합니다.
	PlayGunFireMontage();

	// CrosshairFireTimer를 시작합니다.
	StartCrosshairFireTimer();

	// AutoFireTimer를 시작합니다.
	StartAutoFireTimer();
}

const bool AShooterCharacter::GetBeamEndPoint(
	const FVector& MuzzleSocketLocation,
	FVector& OutBeamEndPoint)
{
	// 1차 트레이스를 실시합니다.
	FHitResult CrosshairHitResult;
	FVector CrosshairHitLocation;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, CrosshairHitLocation);

	if (bCrosshairHit)
	{
		// 1차 트레이스 Hit에 성공하여 2차 트레이스를 실시합니다.
		// Gun Barrel에서부터 실제로 조준한 방향으로 LineTrace를 실행하여 부딪친 물체가 있는지 판별합니다.
		const FVector WeaponTraceStart{MuzzleSocketLocation};
		const FVector WeaponTraceDirection{(CrosshairHitLocation - MuzzleSocketLocation).GetSafeNormal()};
		// 1차 Trace로 검출한 지점까지 2차 Trace를 실시할 경우
		// 아주 작은 길이의 차이로 검출될 수 없는 문제가 생길 수 있기 때문에, 약간의 길이를 더합니다.
		const FVector WeaponTraceEnd{CrosshairHitLocation + WeaponTraceDirection * 1.25f};

		FHitResult WeaponTraceHit;
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

		// 최종적으로 Gun Barrel으로부터 실시한 LineTrace가 반환되는 결과값으로 사용됩니다.
		if (WeaponTraceHit.bBlockingHit)
		{
			OutBeamEndPoint = WeaponTraceHit.Location;
			return true;
		}
	}

	OutBeamEndPoint = CrosshairHitLocation;
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpolation(float DeltaTime)
{
	// Camera의 FOV를 TargetFOV값으로 서서히 다가가도록 보간합니다.
	const float TargetFOV = bAiming ? CameraZoomFOV : CameraDefaultFOV;
	CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, TargetFOV, DeltaTime, ZoomInterpSpeed);
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	BaseTurnRate = bAiming ? AimingTurnRate : HipTurnRate;
	BaseLookUpRate = bAiming ? AimingLookUpRate : HipLookUpRate;
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	const FVector2D WalkSpeedRange{0.f, GetCharacterMovement()->MaxWalkSpeed};
	const FVector2D VelocityMultiplierRange{0.f, 1.f};
	FVector VelocityXY = GetVelocity();
	VelocityXY.Z = 0;

	// 캐릭터가 이동중일 때 이동 속도에 따라 Crosshair가 벌어지는 정도를 갱신합니다.
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, VelocityXY.Size());

	// 캐릭터가 공중에 떠 있을 때 Crosshair가 벌어지는 정도를 갱신합니다.
	CrosshairInAirFactor = GetCharacterMovement()->IsFalling()
		                       // 공중에 떠있을 때 서서히 Crosshair가 벌어지기 시작합니다.
		                       ? FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f)
		                       // 공중에 떠있다가 땅에 착지했을 때 Crosshair가 굉장히 빠른 속도로 원래대로 좁혀집니다.
		                       : FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);

	// 캐릭터가 조준중일 때 Crosshair가 벌어지는 정도를 갱신합니다.
	CrosshairAimFactor = bAiming
		                     // 조준중일 때 Crosshair는 절대로 벌어지지 않습니다.
		                     ? FMath::FInterpTo(CrosshairAimFactor, -5.f, DeltaTime, 50.f)
		                     // 조준을 해제할 때 Crosshair는 서서히 원래대로 커집니다.
		                     : FMath::FInterpTo(CrosshairAimFactor, 0, DeltaTime, 10.f);

	CrosshairFiringFactor = bFiring
		                        ? FMath::FInterpTo(CrosshairFiringFactor, .3f, DeltaTime, 60.f)
		                        : FMath::FInterpTo(CrosshairFiringFactor, 0.f, DeltaTime, 60.f);

	CrosshairSpreadMultiplier = FMath::Clamp(
		1.f
		+ CrosshairVelocityFactor
		+ CrosshairInAirFactor
		+ CrosshairAimFactor
		+ CrosshairFiringFactor,
		0.f, BIG_NUMBER);
}

void AShooterCharacter::StartCrosshairFireTimer()
{
	bFiring = true;

	GetWorld()->GetTimerManager().SetTimer(CrosshairFireTimer, this, &AShooterCharacter::OnFinishedCrosshairFireTimer, FireTimerDuration);
}

void AShooterCharacter::OnFinishedCrosshairFireTimer()
{
	bFiring = false;
}

void AShooterCharacter::OnFireButtonPressed()
{
	bFireButtonPressed = true;

	FireWeapon();
}

void AShooterCharacter::OnFireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartAutoFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::OnEndAutoFireTimer, AutoFireRate);
}

void AShooterCharacter::OnEndAutoFireTimer()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo() == false)
	{
		ReloadWeapon();
		return;
	}

	if (bFireButtonPressed)
		FireWeapon();
}

const bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// 현재 Viewport의 크기를 얻어오고, Viewport의 중심에 있는 Crosshair의 위치를 계산합니다.
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
		GEngine->GameViewport->GetViewportSize(ViewportSize);

	FVector2D CrosshairViewportPosition{ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f};

	// Crosshair의 World Position과 Direction을 얻습니다.
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairViewportPosition, CrosshairWorldPosition, CrosshairWorldDirection);

	// Viewport 좌표 -> World 좌표로 변환이 성공했다면 LineTrace를 실시합니다.
	if (bScreenToWorld)
	{
		const FVector ScreenTraceStart{CrosshairWorldPosition};
		const FVector ScreenTraceEnd{CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f};
		GetWorld()->LineTraceSingleByChannel(OutHitResult, ScreenTraceStart, ScreenTraceEnd, ECollisionChannel::ECC_Visibility);

		// LineTrace을 실시한 결과 충돌된 무언가가 있다면 OutHitLocation으로 그 충돌 지점을 반환합니다.
		// 그렇지 않다면 ScreenTraceEnd를 반환합니다.
		OutHitLocation = OutHitResult.bBlockingHit ? OutHitResult.Location : ScreenTraceEnd;

		// LineTrace에 충돌한 무언가가 있는지에 대한 여부를 반환합니다.
		return OutHitResult.bBlockingHit;
	}

	return false;
}

void AShooterCharacter::TraceForItems()
{
	// Trace를 앞서 TraceHitLastFrame과 TraceHitItem을 초기화합니다.
	TraceHitItemLastFrame = TraceHitItem;
	TraceHitItem = nullptr;


	// 아무런 아이템도 Trace되어야 하지 않는 상태에서는
	// 어떠한 아이템의 Widget도 표시하지 않습니다.
	if (GetShouldForItemTrace() == false)
	{
		if (TraceHitItemLastFrame != nullptr)
		{
			TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
			TraceHitItemLastFrame = nullptr;
		}

		return;
	}

	// 아이템과 곂친 상태일 때만 아래 코드가 실행됩니다.

	FHitResult ItemTraceResult;
	FVector ItemHitLocation;
	TraceUnderCrosshairs(ItemTraceResult, ItemHitLocation);

	// 바라보고 있는 물체가 있으며
	// 그 물체가 Widget을 가진 아이템일 때에 Widget을 표시합니다.
	if (ItemTraceResult.bBlockingHit)
	{
		FString Message = FString::Printf(TEXT("Item Trace Hit : %s %s"), *ItemTraceResult.GetActor()->GetName(), *ItemTraceResult.GetComponent()->GetName());
		GEngine->AddOnScreenDebugMessage(10, 0, FColor::White, Message);

		TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
		if (TraceHitItem && TraceHitItem->GetPickupWidget())
		{
			TraceHitItem->GetPickupWidget()->SetVisibility(true);

			// 가장 마지막으로 Trace했던 무기와 현재 프레임에 발견한 무기가 다르다면
			// 가장 마지막으로 Trace했던 무기의 Widget을 숨깁니다.
			if (TraceHitItemLastFrame && TraceHitItemLastFrame != TraceHitItem)
				TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);

			return;
		}
	}

	// 바라보고 있는 아이템이 없다면 어떠한 아이템의 Widget도 표시하지 않습니다.
	if (TraceHitItemLastFrame != nullptr)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame = nullptr;
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon() const
{
	// 기본 장착될 무기의 클래스가 정해져있는지 확인합니다.
	if (DefaultWeaponClass == nullptr)
		return nullptr;

	// 무기를 스폰합니다.
	AWeapon* DefaultWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);

	return DefaultWeapon;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr)
		return;

	// Hand 소켓을 얻어옵니다.
	const USkeletalMeshSocket* RightHandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket == nullptr)
		return;

	// Hand 소켓에 스폰한 무기를 붙입니다.
	RightHandSocket->AttachActor(WeaponToEquip, GetMesh());

	// WeaponToEquipped을 장착합니다.
	EquippedWeapon = WeaponToEquip;
	WeaponToEquip->SetItemState(EItemState::EIS_Equipped);
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon == nullptr)
		return;

	EquippedWeapon->BeDropped();
	EquippedWeapon = nullptr;
}

void AShooterCharacter::InteractButtonPressed()
{
	if (TraceHitItem)
	{
		TraceHitItem->StartItemInterp(this);
	}
}

void AShooterCharacter::InteractButtonReleased()
{
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr)
		return false;

	return EquippedWeapon->GetAmmoCount() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if (FireSound)
		UGameplayStatics::PlaySound2D(this, FireSound);
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetMesh());

		if (MuzzleFlash)
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);

		FVector BeamEndPoint;
		const bool bBeamEnd = GetBeamEndPoint(SocketTransform.GetLocation(), BeamEndPoint);
		if (bBeamEnd)
		{
			if (ImpactParticles)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEndPoint);

			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
				if (Beam)
					Beam->SetVectorParameter(FName{"Target"}, BeamEndPoint);
			}
		}
	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::OnReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied)
		return;

	if (CarryingAmmo())
	{
		CombatState = ECombatState::ECS_Reloading;

		// Reload 애니메이션을 재생하고, 무기 타입에 맞는 Section으로 건너뛴다.
		const FName MontageSectionName = EquippedWeapon->GetReloadMontageSectionName();
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(MontageSectionName);
		}
	}

	CombatState = ECombatState::ECS_Reloading;
}

void AShooterCharacter::OnFinishedReloading()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (EquippedWeapon == nullptr)
		return;

	if (CarryingAmmo() == false)
		return;

	const EAmmoType AmmoType = EquippedWeapon->GetAmmoType();
	const int32 CarriedAmmo = AmmoMap[AmmoType];
	const int32 MagazineEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmoCount();

	// 탄창에 넣을 총알의 개수를 계산합니다.
	const int32 ReloadAmount = CarriedAmmo < MagazineEmptySpace ? CarriedAmmo : MagazineEmptySpace;

	// 소지한 탄약의 개수를 줄이고, 탄창에 탄약을 추가합니다.
	AmmoMap[AmmoType] -= ReloadAmount;
	EquippedWeapon->ReloadAmmo(ReloadAmount);
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr)
		return false;

	EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType) == false)
		return false;

	return AmmoMap[AmmoType] > 0;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpolation(DeltaTime);

	SetLookRates();

	CalculateCrosshairSpread(DeltaTime);

	TraceForItems();
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::OnFireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Released, this, &AShooterCharacter::OnFireButtonReleased);

	PlayerInputComponent->BindAction("Aiming", EInputEvent::IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("Aiming", EInputEvent::IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Interact", EInputEvent::IE_Pressed, this, &AShooterCharacter::InteractButtonPressed);
	PlayerInputComponent->BindAction("Interact", EInputEvent::IE_Released, this, &AShooterCharacter::InteractButtonReleased);

	PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &AShooterCharacter::OnReloadButtonPressed);
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		SwapWeapon(Weapon);
	}
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

FVector AShooterCharacter::GetItemInterpDesiredDestination() const
{
	const FVector CameraWorldLocation{FollowCamera->GetComponentLocation()};
	const FVector CameraForward{FollowCamera->GetForwardVector()};
	const FVector CameraRight{FollowCamera->GetRightVector()};

	return CameraWorldLocation
		+ CameraForward * ItemInterpDesiredOffset.X
		+ CameraRight * ItemInterpDesiredOffset.Y
		+ FVector{0.f, 0.f, ItemInterpDesiredOffset.Z};
}
