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
	BaseTurnRate(45.0f),
	BaseLookUpRate(45.0f)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// CameraBoom 생성 (
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true; // Controller에 따라 Arm을 회전시킵니다.
	CameraBoom->SocketOffset = FVector{0.f, 50.f, 50.f}; // 카메라가 캐릭터의 살짝 옆에서 비추도록 offset을 설정합니다.

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

	UE_LOG(LogTemp, Warning, TEXT("BeginPlay() called!"));
	int myInt{42};
	UE_LOG(LogTemp, Warning, TEXT("int myInt: %d"), myInt);

	float myFloat{3.14159f};
	UE_LOG(LogTemp, Warning, TEXT("float myFloat: %f"), myFloat);

	double myDouble{0.000756};
	UE_LOG(LogTemp, Warning, TEXT("double myDouble: %lf"), myDouble);

	char myChar{'3'};
	UE_LOG(LogTemp, Warning, TEXT("char myChar: %c"), myChar);

	wchar_t myWideChar{L'4'};
	UE_LOG(LogTemp, Warning, TEXT("w_char myWideChar: %lc"), myWideChar);

	bool myBool{true};
	UE_LOG(LogTemp, Warning, TEXT("bool myBool: %d"), myBool);

	UE_LOG(LogTemp, Warning, TEXT("int %d, float: %f, bool: %d"), myInt, myFloat, myBool);

	FString myString{TEXT("My String!!!!")};
	UE_LOG(LogTemp, Warning, TEXT("FString myString: %s"), *myString);

	UE_LOG(LogTemp, Warning, TEXT("Name of instance: %s"), *GetName());
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

void AShooterCharacter::FireWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("Fire Weapon"));

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
			const FVector Start{CrosshairWorldPosition};
			const FVector End{CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f};

			FVector BeamEndPoint{End};

			// 조준한 방향으로 LineTrace를 실행하여 부딪친 물체가 있는지 판별합니다.
			FHitResult ScreenTraceHit;
			GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
			if (ScreenTraceHit.bBlockingHit)
			{
				BeamEndPoint = ScreenTraceHit.Location;
			}

			// 두번째 Trace를 실시합니다.
			// 이번에는 Gun Barrel에서부터 시작합니다.
			const FVector WeaponTraceStart{SocketTransform.GetLocation()};
			const FVector WeaponTraceEnd{BeamEndPoint};

			FHitResult WeaponTraceHit;
			GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
			if (WeaponTraceHit.bBlockingHit)
			{
				BeamEndPoint = WeaponTraceHit.Location;

				if (ImpactParticles)
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEndPoint);
			}

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

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::FireWeapon);
}
