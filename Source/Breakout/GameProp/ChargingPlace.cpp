// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/ChargingPlace.h"
#include "Components/BoxComponent.h"
#include "Character/CharacterBase.h"
#include "Weapon/WeaponBase.h"
#include "Player/CharacterController.h"
// Sets default values
AChargingPlace::AChargingPlace()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ChargingPlaceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AircraftMesh"));
	SetRootComponent(ChargingPlaceMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AChargingPlace::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AChargingPlace::OnBoxOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AChargingPlace::OnBoxEndOverlap);
}

void AChargingPlace::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	InCh = Cast<ACharacterBase>(OtherActor);
	if (InCh)
	{
		InChWeapon = InCh->GetCurWeapon();
		if (InChWeapon)
		{
			bInCh = true;
		}
	}
}

void AChargingPlace::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<ACharacterBase>(OtherActor))
	{
		bInCh = false;
	}
}

// Called every frame
void AChargingPlace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bInCh)
	{
		ChargingTime += DeltaTime;
		if (ChargingTime >= 2.f)
		{
			InChWeapon->CurAmmo += 1;
			if (InChWeapon->CurAmmo >= InChWeapon->MaxAmmo)
				InChWeapon->CurAmmo = InChWeapon->MaxAmmo;

			InCh->GetChController()->SetHUDAmmo(InChWeapon->CurAmmo);
			ChargingTime = 0.f;
		}
		//InChWeapon->CurAmmo = FMath::Clamp(InChWeapon->CurAmmo, 0, InChWeapon->MaxAmmo);
	}
}

