// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AShooterPlayerController();

protected:
	virtual void BeginPlay() override;

private:
	/**
	 * @brief 화면 전체를 덮는 HUD Overlay Blueprint 클래스입니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Widgets", meta = (AllowPrivateAccess="true"))
	TSubclassOf<class UUserWidget> HUDOverlayClass;

	/**
	 * @brief 생성한 HUD Overlay Widget Blueprint에 대한 레퍼런스입니다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Widgets", meta = (AllowPrivateAccess="true"))
	class UUserWidget* HUDOverlay;
};
