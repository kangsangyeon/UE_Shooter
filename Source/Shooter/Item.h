// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

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
	 * @brief 플레이어가 아이템을 바라볼 때 나타날 Popup widget입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta=(AllowPrivateAccess="true"))
	class UWidgetComponent* PickupWidget;

public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
};
