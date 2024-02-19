
#include "Skill/SkillComponent.h"
#include "Character/CharacterBase.h"
#include "Player/CharacterController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

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
	
	GhostFX = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Niagara/DashFX.DashFX"));

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
		DashPoint = 3;
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

	if (CurSelectedSKill == ESelectedSkill::E_Skill1)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("RecordedTime : %f"), RecordedTime));

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
		GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("DashPoint : %d"), DashPoint));
		if(DashPoint < 3) DashCoolChargeTime += DeltaTime;
		if (DashPoint < 3 && DashCoolChargeTime>=4.f)
		{
			DashPoint += 1;
			DashCoolChargeTime = 0.f;
		}
		//DashStart();
	}
	else if (CurSelectedSKill == ESelectedSkill::E_Skill3)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("RecordedGhostTime : %f"), RecordedGhostTime));
		GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("GhostCoolChargeTime : %f"), GhostCoolChargeTime));
		if (bCoolTimeFinish && bGhost)
		{
			GhostStart();
			RecordedGhostTime += DeltaTime;
			if (RecordedGhostTime >= 5.f) GhostEnd();
		}

		if (!bCoolTimeFinish)
		{
			GhostCoolChargeTime += DeltaTime;
			if (GhostCoolChargeTime >= 10.f) 
			{ 
				bCoolTimeFinish = true; 
				GhostCoolChargeTime = 0.f;
			}
		}
	}
	else if(CurSelectedSKill == ESelectedSkill::E_Skill4)
	{ 
		GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("RecordedTelpoTime : %f"), GhostCoolChargeTime));
		if (!TelepoChargeTime)
		{
			GhostCoolChargeTime += DeltaTime;
			if (GhostCoolChargeTime >= 15.f)
			{
				TelepoChargeTime = true;
			}
		}

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
void USkillComponent::DashStart()
{
	//const FRotator Rotation = Controller->GetControlRotation();
	//const FRotator YawRotaion(0.f, Rotation.Yaw, 0.f);
	//const FVector ForwardDir = FRotationMatrix(YawRotaion).GetUnitAxis(EAxis::X);

	if (bDash && bCoolTimeFinish)
	{
		UE_LOG(LogTemp, Log, TEXT("DASHPOINT --"));
		bDash = false;
		bCoolTimeFinish = false;
		DashPoint -= 1;
		OldVelocity = Character->GetMovementComponent()->Velocity;
		Character->GetMovementComponent()->Velocity = //ForwardDir * 5000.f;
			Character->GetActorForwardVector() * 20000.f;
		GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &USkillComponent::FinishDashTimer, 0.5, false);
	}
}

void USkillComponent::FinishDashTimer()
{
	Character->GetMovementComponent()->Velocity = OldVelocity;
	GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &USkillComponent::CoolTimeDashTimer, 0.2, false);
}

void USkillComponent::CoolTimeDashTimer()
{
	bCoolTimeFinish = true;
}

//Skill3
void USkillComponent::GhostStart()
{
	if (bGhost && bCoolTimeFinish)
	{
		OldVelocity = Character->GetMovementComponent()->Velocity;
		Character->GetMovementComponent()->Velocity = Character->GetActorForwardVector() * 2500.f;
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);

		if (GhostFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GhostFX, Character->GetActorLocation(), Character->GetActorRotation());
		}
	}
}

void USkillComponent::GhostEnd()
{
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	Character->GetMovementComponent()->Velocity = OldVelocity;
	bCoolTimeFinish = false;
	bGhost = false;
	RecordedGhostTime = 0.f;
}

void USkillComponent::SaveCurLocation()
{
	if ( TelepoChargeTime)
	{
		UE_LOG(LogTemp, Log, TEXT("START"));
		SavedLocation=Character->GetActorLocation();
		bSaved = true;
		GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &USkillComponent::SetCanTelepo, 1, false);

		//UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),);
	}

}

void USkillComponent::SetLocation()
{
	if (bSaved && CanTelepo)
	{
		UE_LOG(LogTemp, Log, TEXT("End"));
		TelepoChargeTime = false;
		bSaved = false;
		Character->SetActorLocation(SavedLocation);
		Toggle += 1;
	}

}
