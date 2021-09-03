// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
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
	CrosshairShootingFactor(0.f)
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
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

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

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

const bool AShooterCharacter::GetBeamEndPoint(
	const FVector& MuzzleSocketLocation,
	FVector& OutBeamEndPoint)
{
	// 현재 Viewport의 크기를 얻어오고, Viewport의 중심에 있는 Crosshair의 위치를 계산합니다.
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
		GEngine->GameViewport->GetViewportSize(ViewportSize);

	FVector2D CrosshairViewportPosition{ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f};
	CrosshairViewportPosition.Y -= 50.f;

	// Crosshair의 World Position과 Direction을 얻습니다.
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairViewportPosition, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FVector BeamEndPoint;

		// 1차 트레이스를 실시합니다.
		// 크로스헤어가 존재하는 가상 지점인 World Position에서 조준하는 방향으로 트레이스합니다.
		// 이 트레이스 결과로 실제로 사용자가 조준한 지점을 알아낼 수 있습니다.
		const FVector ScreenTraceStart{CrosshairWorldPosition};
		const FVector ScreenTraceEnd{CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f};
		BeamEndPoint = ScreenTraceEnd;

		FHitResult ScreenTraceHit;
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, ScreenTraceStart, ScreenTraceEnd, ECollisionChannel::ECC_Visibility);
		if (ScreenTraceHit.bBlockingHit)
		{
			BeamEndPoint = ScreenTraceHit.Location;

			// 1차 트레이스 Hit에 성공하여 2차 트레이스를 실시합니다.
			// Gun Barrel에서부터 실제로 조준한 방향으로 LineTrace를 실행하여 부딪친 물체가 있는지 판별합니다.
			const FVector WeaponTraceStart{MuzzleSocketLocation};
			const FVector WeaponTraceEnd{BeamEndPoint};

			FHitResult WeaponTraceHit;
			GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
			if (WeaponTraceHit.bBlockingHit)
				BeamEndPoint = WeaponTraceHit.Location;
		}

		OutBeamEndPoint = BeamEndPoint;
		return true;
	}

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

	CrosshairSpreadMultiplier = FMath::Clamp(
		1.f
		+ CrosshairVelocityFactor
		+ CrosshairInAirFactor
		+ CrosshairAimFactor,
		0.f, BIG_NUMBER);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpolation(DeltaTime);

	SetLookRates();

	CalculateCrosshairSpread(DeltaTime);
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

	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::FireWeapon);

	PlayerInputComponent->BindAction("Aiming", EInputEvent::IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("Aiming", EInputEvent::IE_Released, this, &AShooterCharacter::AimingButtonReleased);
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}
