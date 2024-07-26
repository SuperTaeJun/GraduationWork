// Fill out your copyright notice in the Description page of Project Settings.


#include "InterObject/ExplosiveActor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include"Character/CharacterBase.h"

AExplosiveActor::AExplosiveActor()
{
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);
}
void AExplosiveActor::BeginPlay()
{
	Super::BeginPlay();
	OnTakeAnyDamage.AddDynamic(this, &AExplosiveActor::ReciveDamage);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AExplosiveActor::OnBeginSphereOverlap);
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AExplosiveActor::OnEndSphereOverlap);
	HP = 30.f;
}

void AExplosiveActor::ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	HP -= Damage;
	if (HP <= 0)
	{
		//AController* FiringController = InstigatorController;
		//if (FiringController)
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("EXPLOSIVE"));
		//	//bool check =UGameplayStatics::ApplyRadialDamage
		//	//(
		//	//	GetWorld(), // World context object
		//	//	40.f, // BaseDamage
		//	//	GetActorLocation(), // Origin
		//	//	500.f, // DamageRadius
		//	//	UDamageType::StaticClass(), // DamageTypeClass
		//	//	TArray<AActor*>(), // IgnoreActors
		//	//	nullptr, // DamageCauser
		//	//	nullptr // InstigatorController
		//	//);


		//}
		if (InCharacter)
		{
			InCharacter->ReciveDamage(nullptr, 30.f, nullptr, nullptr, nullptr);
		}
		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
		}
		Destroy();
	}
}

void AExplosiveActor::OnBeginSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacterBase* Temp = Cast<ACharacterBase>(OtherActor);
	if (Temp)
		InCharacter = Temp;
}

void AExplosiveActor::OnEndSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACharacterBase* Temp=Cast<ACharacterBase>(OtherActor);
	if(Temp)
		InCharacter = nullptr;
}

void AExplosiveActor::Destroyed()
{
	Super::Destroyed();
	if (ImpactNiagara)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagara, GetActorLocation());

}

