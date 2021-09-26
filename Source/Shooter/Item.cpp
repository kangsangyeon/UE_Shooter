// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
AItem::AItem() :
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup),
	ItemZCurveTime(0.7f),
	ItemInterpStartLocation(FVector::ZeroVector),
	ItemInterpTargetLocation(FVector::ZeroVector),
	bInterping(false),
	ItemInterpX(0.f),
	ItemInterpY(0.f)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(Mesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(Mesh);
	AreaSphere->SetSphereRadius(150.f);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(Mesh);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	// Pickup Widget은 플레이어가 가까이 다가갔을 때에만 보여져야 합니다.
	// 따라서 기본적으로 숨김 처리합니다.	
	PickupWidget->SetVisibility(false);

	// AreaSphere의 충돌 콜백을 설정합니다.
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnAreaSphereBeginOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnAreaSphereEndOverlap);

	// Item의 상태에 따라 속성을 설정합니다.
	SetItemProperties(ItemState);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 아이템이 EquipInterping 상태일 때 Interp합니다.
	ItemInterp(DeltaTime);
}

void AItem::OnAreaSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor == nullptr)
		return;

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter == nullptr)
		return;

	ShooterCharacter->IncreaseOverlappedItemCount(1);
}

void AItem::OnAreaSphereEndOverlap(UPrimitiveComponent* OverllapedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr)
		return;

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter == nullptr)
		return;

	ShooterCharacter->IncreaseOverlappedItemCount(-1);
}

void AItem::SetEnableCollision(bool Enable)
{
	ECollisionEnabled::Type EnabledType = Enable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision;

	CollisionBox->SetCollisionEnabled(EnabledType);
	AreaSphere->SetCollisionEnabled(EnabledType);
}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:
		SetItemPropertiesPickupState();
		break;
	case EItemState::EIS_Equipped:
		SetItemPropertiesEquippedState();
		break;
	case EItemState::EIS_Falling:
		SetItemPropertiesFallingState();
		break;
	case EItemState::EIS_EquipInterping:
		SetItemPropertiesEquipInterpingState();
		break;
	}
}

void AItem::BeDropped()
{
	// 아이템을 장착되어 있던 부모 컴포넌트로부터 떼어냅니다.
	FDetachmentTransformRules DetachmentTransformRules{EDetachmentRule::KeepWorld, true};
	Mesh->DetachFromComponent(DetachmentTransformRules);

	// 아이템은 그대로 떨어지는 상태가 됩니다.
	SetItemState(EItemState::EIS_Falling);
}

void AItem::StartItemInterp(AShooterCharacter* Char)
{
	this->Character = Char;

	bInterping = true;
	ItemInterpStartLocation = GetActorLocation();
	SetItemState(EItemState::EIS_EquipInterping);

	GetWorld()->GetTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishItemInterp, ItemZCurveTime);
}

void AItem::FinishItemInterp()
{
	if (Character)
		Character->GetPickupItem(this);

	bInterping = false;
}

void AItem::ItemInterp(float DeltaTime)
{
	if (bInterping == false)
		return;

	if (Character == nullptr || ItemZCurve == nullptr)
		return;

	// 이번 프레임에 이동할 목적지 Z 위치를 계산합니다.
	const FVector ItemInterpDesiredLocation = FVector{Character->GetItemInterpDesiredDestination()};
	const float ItemToCameraZ = (ItemInterpDesiredLocation - ItemInterpStartLocation).Z;

	const float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(ItemInterpTimer);
	const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

	const float InterpZValue = ItemInterpStartLocation.Z + ItemToCameraZ * CurveValue;

	// 이번 프레임에 이동할 목적지 X Y 위치를 계산합니다.
	const FVector CurrentLocation = GetActorLocation();
	const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, ItemInterpDesiredLocation.X, DeltaTime, 30.f);
	const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, ItemInterpDesiredLocation.Y, DeltaTime, 30.f);

	// 액터의 위치를 설정합니다.
	const FVector ItemLocation = FVector{InterpXValue, InterpYValue, InterpZValue};
	SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);
}

void AItem::SetItemPropertiesPickupState()
{
	// Mesh의 속성을 설정합니다.
	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
	Mesh->SetVisibility(true);
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// AreaSphere의 속성을 설정합니다.
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// CollisionBox의 속성을 설정합니다.
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AItem::SetItemPropertiesEquipInterpingState()
{
	// Mesh의 속성을 설정합니다.
	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
	Mesh->SetVisibility(true);
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// AreaSphere의 속성을 설정합니다.
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	// CollisionBox의 속성을 설정합니다.
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// PickupWidget의 속성을 설정합니다.
	PickupWidget->SetVisibility(false);
}

void AItem::SetItemPropertiesPickedUpState()
{
}

void AItem::SetItemPropertiesEquippedState()
{
	// Mesh의 속성을 설정합니다.
	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
	Mesh->SetVisibility(true);
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// AreaSphere의 속성을 설정합니다.
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	// CollisionBox의 속성을 설정합니다.
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// PickupWidget의 속성을 설정합니다.
	PickupWidget->SetVisibility(false);
}

void AItem::SetItemPropertiesFallingState()
{
	// Mesh의 속성을 설정합니다.
	Mesh->SetSimulatePhysics(true);
	Mesh->SetEnableGravity(true);
	Mesh->SetVisibility(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

	// AreaSphere의 속성을 설정합니다.
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// CollisionBox의 속성을 설정합니다.
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

TArray<bool> AItem::GetActiveStarsOfRarity() const
{
	TArray<bool> ActiveStars;
	for (int i = 0; i <= 5; i++)
		ActiveStars.Add(false);

	switch (ItemRarity)
	{
	case EItemRarity::EIR_Legendary:
		ActiveStars[5] = true;
	case EItemRarity::EIR_Epic:
		ActiveStars[4] = true;
	case EItemRarity::EIR_Rare:
		ActiveStars[3] = true;
	case EItemRarity::EIR_Common:
		ActiveStars[2] = true;
	case EItemRarity::EIR_Damaged:
		ActiveStars[1] = true;

		// 배열의 [0] 요소는 항상 false로 설정되어있으며, 사용하지 않을 것입니다.
	}

	return ActiveStars;
}
