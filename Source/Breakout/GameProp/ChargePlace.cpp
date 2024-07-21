
#include "GameProp/ChargePlace.h"
#include "Components/BoxComponent.h"
#include "Character/CharacterBase.h"

AChargePlace::AChargePlace()
{

	PrimaryActorTick.bCanEverTick = true;

	ChargeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChargeMesh"));
	SetRootComponent(ChargeMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
}

void AChargePlace::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AChargePlace::OnBoxOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AChargePlace::OnBoxEndOverlap);
}

void AChargePlace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (bInCh && InCh && InCh->GetMainController())
	{
		UE_LOG(LogTemp, Warning, TEXT("CHARGING"));
		InCh->SetHealth(InCh->GetHealth() + (DeltaTime*0.5f));
		InCh->UpdateHpHUD();
	}
}

void AChargePlace::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacterBase* Temp= Cast<ACharacterBase>(OtherActor);
	if (Temp)
	{
		UE_LOG(LogTemp, Warning, TEXT("TEMP"));
		InCh = Temp;
		bInCh = true;
	}
}

void AChargePlace::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	InCh = nullptr;
	bInCh = false;
}

