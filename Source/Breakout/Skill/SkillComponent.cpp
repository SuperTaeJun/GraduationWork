
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
			Controller = Character->GetController();
		}
	}

}
void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	switch (CurSelectedSKill)
	{
	case ESelectedSkill::E_Skill1:
		MaxSaveTime = 5.f;
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

void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("RecordedTime : %f"), RecordedTime));

	if (CurSelectedSKill == ESelectedSkill::E_Skill1)
	{
		if (!bTimeReplay)
		{
			StoreFrameData(DeltaTime);
		}
		else if (!bOutOfData)
		{
			Replay(DeltaTime);
		}
	}
	else if(CurSelectedSKill == ESelectedSkill::E_Skill2)
	{
	}
	else if (CurSelectedSKill == ESelectedSkill::E_Skill3)
	{

	}
	else if(CurSelectedSKill == ESelectedSkill::E_Skill4)
	{ 
	}
	
}


//Skill1
void USkillComponent::StoreFrameData(float DeltaTime)
{
	RunningTime = 0.f;
	LeftRunningTime = 0.f;
	RightRunningTime = 0.f;
	Temp += DeltaTime;
	FCharacterFrameData Package(Character->GetActorLocation(), Temp);
	if (Temp >= 0.2f)
	{
		if (RecordedTime < MaxSaveTime)
		{
			UE_LOG(LogTemp, Log, TEXT("RECORDETIME"));
			FrameDatas.AddTail(Package);
			RecordedTime += Package.DeltaTime;
			bOutOfData = false;
		}
		else
		{
			while (RecordedTime >= MaxSaveTime)
			{
				auto Head = FrameDatas.GetHead();
				float HeadDeltaTime = Head->GetValue().DeltaTime;
				FrameDatas.RemoveNode(Head);
				RecordedTime -= HeadDeltaTime;
			}
			FrameDatas.AddTail(Package);
			RecordedTime += Package.DeltaTime;
			bOutOfData = false;
		}
		Temp = 0.f;
	}
}
void USkillComponent::Replay(float DeltaTime)
{
	RunningTime += DeltaTime*4;
	auto Right = FrameDatas.GetTail();
	auto Left = Right->GetPrevNode();
	LeftRunningTime = RightRunningTime + Right->GetValue().DeltaTime;

	while (RunningTime > LeftRunningTime)
	{
		RightRunningTime += Right->GetValue().DeltaTime;
		Right = Left;
		LeftRunningTime += Left->GetValue().DeltaTime;
		Left = Left->GetPrevNode();

		auto Tail = FrameDatas.GetTail();
		RecordedTime -= Tail->GetValue().DeltaTime;
		FrameDatas.RemoveNode(Tail);
		if (Left == FrameDatas.GetHead())
		{
			bOutOfData = true;
		}
	}
	//각수치들 보간
	if (RunningTime <= LeftRunningTime && RunningTime >= RightRunningTime)
	{
		float DT = RunningTime - RightRunningTime;
		float Interval = LeftRunningTime - RightRunningTime;
		float Fraction = DT / Interval;

		FVector InterpLocation = FMath::VInterpTo(Right->GetValue().Location, Left->GetValue().Location, Fraction, 1.f);

		Character->SetActorLocation(InterpLocation);
	}
}

//Skill2