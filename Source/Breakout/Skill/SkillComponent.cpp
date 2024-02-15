
#include "Skill/SkillComponent.h"
#include "Character/CharacterBase.h"
#include "Player/CharacterController.h"

USkillComponent::USkillComponent()
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
void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	switch (CurSelectedSKill)
	{
	case ESelectedSkill::E_Skill1:
		StoreTimer();
		break;
	case ESelectedSkill::E_Skill2:
		break;
	case ESelectedSkill::E_Skill3:
		break;
	case ESelectedSkill::E_Skill4:
		break;
	default:
		break;
	}
}

void USkillComponent::StoreFrameData()
{
	FrameData->Transform.Insert(Character->GetActorTransform(),0);
	if (FrameData->Transform.Num() > FrameData->MaxTransfirm)
	{
		FrameData->Transform.SetNum(FrameData->MaxTransfirm);
	}
}

void USkillComponent::StoreTimer()
{
	GetWorld()->GetTimerManager().SetTimer(Skill1Timer, this, &USkillComponent::StoreFrameData, 1.f);
}

void USkillComponent::Recall()
{
	if (Cnt <= FrameData->Transform.Num() - 1)
	{
		const FTransform ActorTransform = Character->GetActorTransform();
		const FTransform DataTransform = FrameData->Transform[Cnt];
		const FTransform LerpTransform;//FMath::Lerp<FTransform>(ActorTransform, DataTransform,  GetWorld()->GetTimeSeconds());
		Character->SetActorTransform(LerpTransform);

		++Cnt;
	}
	else
	{
		StoreTimer();
		Cnt = 0;
	}
}


// Called every frame
void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	switch (CurSelectedSKill)
	{
	case ESelectedSkill::E_Skill1:
		//UE_LOG(LogTemp, Log, TEXT("SKILL"));
		break;
	case ESelectedSkill::E_Skill2:
		break;
	case ESelectedSkill::E_Skill3:
		break;
	case ESelectedSkill::E_Skill4:
		break;
	default:
		break;
	}
	// ...
}

