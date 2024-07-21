// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/DetectItem.h"
#include"Character/CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
void ADetectItem::BeginPlay()
{
	Super::BeginPlay();
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ADetectItem::OnBoxOverlap);
}
void ADetectItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
void ADetectItem::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT(" OVERLAP"));
	ACharacterBase* OverlapCh = Cast<ACharacterBase>(OtherActor);

	if (OverlapCh)
	{
		UE_LOG(LogTemp, Warning, TEXT(" OVERLAP2"));
		TArray<AActor*> Characters;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterBase::StaticClass(), Characters);
		for (AActor* Character : Characters)
		{
			if(OverlapCh != Character)
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DetectNiagara, Character->GetActorLocation());
		}

		Destroy();
	}
}

