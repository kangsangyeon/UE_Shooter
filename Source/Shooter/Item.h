// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

private:
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
	 * @brief 아이템의 희소성입니다. Pickup Widget에 표시될 별의 개수를 결정합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess="true"))
	EItemRarity ItemRarity;

public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }

	UFUNCTION(BlueprintCallable)
	TArray<bool> GetActiveStarsOfRarity() const;
};
