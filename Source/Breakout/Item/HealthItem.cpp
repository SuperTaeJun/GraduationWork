// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/HealthItem.h"
#include"Character/CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
void AHealthItem::BeginPlay()
{
	Super::BeginPlay();
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AHealthItem::OnBoxOverlap);
}

void AHealthItem::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT(" OVERLAP"));
	ACharacterBase* OverlapCh = Cast<ACharacterBase>(OtherActor);

	if (OverlapCh)
	{
		OverlapCh->SetMaxHealth(OverlapCh->GetMaxHealth() + IncreMaxHp);
		OverlapCh->UpdateHpHUD();

		Destroy();
	}
}
