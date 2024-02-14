
#include "Skill/SkillTimeReverse.h"
#include "Character/CharacterBase.h"
#include "Player/CharacterController.h"

USkillTimeReverse::USkillTimeReverse()
{
	PrimaryComponentTick.bCanEverTick = true;

	if (GetOwner())
	{
		Character = Cast<ACharacterBase>(GetOwner());
		if (Character->GetController())
		{
			Controller = Cast<ACharacterController>(Character->GetController());
		}
	}

}


// Called when the game starts
void USkillTimeReverse::BeginPlay()
{
	Super::BeginPlay();

	
}


// Called every frame
void USkillTimeReverse::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UE_LOG(LogTemp, Log, TEXT("SKILL"));
	// ...
}

