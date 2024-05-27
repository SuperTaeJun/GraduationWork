#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChargingPlace.generated.h"

UCLASS()
class BREAKOUT_API AChargingPlace : public AActor
{
	GENERATED_BODY()
	
public:	
	AChargingPlace();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> ChargingPlaceMesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> CollisionBox;

private:
	UFUNCTION()
	virtual void OnBoxOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	UFUNCTION()
	void OnBoxEndOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);


	bool bInCh = false;
	float ChargingTime = 0.f;

	class ACharacterBase* InCh;
	class AWeaponBase* InChWeapon;
};
