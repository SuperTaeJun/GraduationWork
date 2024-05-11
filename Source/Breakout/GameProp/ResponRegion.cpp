// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/ResponRegion.h"
#include "Character/CharacterBase.h"
#include "HUD/MainHUD.h"
#include "Player/CharacterController.h"
#include "Components/SphereComponent.h"
// Sets default values
AResponRegion::AResponRegion()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	SetRootComponent(AreaSphere);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

// Called when the game starts or when spawned
void AResponRegion::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AResponRegion::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AResponRegion::OnSphereEndOverlap);
}

void AResponRegion::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Log, TEXT("OVERLAP"));
	//TObjectPtr<ACharacterBase> CharacterBase = Cast<ACharacterBase>(OtherActor);
	//if (CharacterBase)
	//{
	//	CharacterBase->SetbInRespon(true);
	//}

	Destroy();
}

void AResponRegion::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//TObjectPtr<ACharacterBase> CharacterBase = Cast<ACharacterBase>(OtherActor);
	//if (CharacterBase)
	//{
	//	ACharacterController* MainController = Cast<ACharacterController>(CharacterBase->Controller);
	//	if (MainController)
	//	{
	//		AMainHUD* MainHUD = Cast<AMainHUD>(MainController->GetHUD());
	//		if (MainHUD)
	//		{
	//			FInputModeGameOnly GameOnlyInput;
	//			MainController->SetInputMode(GameOnlyInput);


	//			CharacterBase->SetbInRespon(false);
	//			CharacterBase->SetbShowSelect(false);
	//			MainHUD->RemoveSelectWeapon();
	//			MainController->bShowMouseCursor = false;
	//			MainController->bEnableMouseOverEvents = false;
	//		}
	//	}
	//}
}

// Called every frame
void AResponRegion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

