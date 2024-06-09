#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "InterActorBase.generated.h"

UCLASS()
class BREAKOUT_API AInterActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInterActorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> ActorMesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> CollisionBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraSystem> ImpactNiagara;

	float HP;
};
