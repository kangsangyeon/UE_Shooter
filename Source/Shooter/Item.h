// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Epic UMETA(DisplayName = "Epic"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),

	EIS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief 다른 액터와 곂쳐지기 시작할 때 호출되는 콜백 메소드입니다.
	 */
	UFUNCTION()
	void OnAreaSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/**
	* @brief 곂쳐있던 다른 액터와 더 이상 곂치지 않기 시작할 때 호출되는 콜백 메소드입니다.
	*/
	UFUNCTION()
	void OnAreaSphereEndOverlap(
		UPrimitiveComponent* OverllapedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	/**
	 * @brief Item의 모든 Collision의 Enable을 설정합니다.
	 * 주로 아이템을 획득했을 때 false로 설정하거나, 필드에 떨어져 다시 주울 수 있는 상태로 되돌릴 때 true로 설정합니다.
	 */
	void SetEnableCollision(bool Enable);

	/**
	 * @brief State에 따라 Item의 컴포넌트들의 속성을 변경합니다.
	 */
	void SetItemProperties(EItemState State);

	/**
	 * @brief 부모 액터로부터 떨어지기 위해 사용하는 메소드입니다.
	 * 주로 장착하는 아이템을 떨어뜨릴 때 호출하게 됩니다.
	 */
	virtual void BeDropped();

protected:
	virtual void SetItemPropertiesPickupState();
	virtual void SetItemPropertiesEquipInterpingState();
	virtual void SetItemPropertiesPickedUpState();
	virtual void SetItemPropertiesEquippedState();
	virtual void SetItemPropertiesFallingState();

protected:
	/**
	* @brief Item의 Skeletal Mesh입니다.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess="true"))
	class USkeletalMeshComponent* Mesh;

	/**
	 * @brief Line Trace에 검출되기 위한 Collision입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess="true"))
	class UBoxComponent* CollisionBox;

	/**
	 * @brief 이 Collision에 곂쳐있을 때에만 Item Tracing을 활성화합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta=(AllowPrivateAccess="true"))
	class USphereComponent* AreaSphere;

	/**
	 * @brief 플레이어가 아이템을 바라볼 때 나타날 Popup widget입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess="true"))
	class UWidgetComponent* PickupWidget;

	/**
	 * @brief Pickup Widget에 표시될 아이템의 이름입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess="true"))
	FString ItemName;

	/**
	 * @brief 아이템 개수입니다. (총알, 기타 등..)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess="true"))
	int32 ItemCount;

	/**
	 * @brief 아이템의 희소성입니다. Pickup Widget에 표시될 별의 개수를 결정합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess="true"))
	EItemRarity ItemRarity;

	/**
	 * @brief 아이템의 상태입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess="true"))
	EItemState ItemState;

public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	void SetItemState(EItemState State);

	UFUNCTION(BlueprintCallable)
	TArray<bool> GetActiveStarsOfRarity() const;
};
