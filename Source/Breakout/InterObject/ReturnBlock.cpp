// Fill out your copyright notice in the Description page of Project Settings.


#include "InterObject/ReturnBlock.h"
#include "Components/BoxComponent.h"

AReturnBlock::AReturnBlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

// Sets default values
// Called when the game starts or when spawned
void AReturnBlock::BeginPlay()
{
	Super::BeginPlay();
	CollisionBox->OnComponentHit.AddDynamic(this, &AReturnBlock::OnHit);

}

// Called every frame
void AReturnBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(!bRetrun)
		StoreFrameData(DeltaTime);
	else
		Replay(DeltaTime);
}

void AReturnBlock::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NomalImpulse, const FHitResult& Hit)
{
	GetWorld()->GetTimerManager().SetTimer(RetrunTimer, this, &AReturnBlock::RetrunStart, 5.f, false);
}

void AReturnBlock::StoreFrameData(float DeltaTime)
{
	RunningTime = 0.f;
	LeftRunningTime = 0.f;
	RightRunningTime = 0.f;
	Temp += DeltaTime;
	FrameData Package(GetActorLocation(), GetActorRotation(), Temp);
	if (Temp >= 0.2f)
	{
		if (RecordedTime < MaxSaveTime)
		{
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

void AReturnBlock::Replay(float DeltaTime)
{
	RunningTime += DeltaTime * 4;
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
			bRetrun = false;
		}
	}
	//각수치들 보간
	if (RunningTime <= LeftRunningTime && RunningTime >= RightRunningTime)
	{
		float DT = RunningTime - RightRunningTime;
		float Interval = LeftRunningTime - RightRunningTime;
		float Fraction = DT / Interval;


		FVector InterpLocation = FMath::VInterpTo(Right->GetValue().Location, Left->GetValue().Location, Fraction, 1.f);
		FRotator InterpRotation = FMath::RInterpTo(Right->GetValue().Rotation, Left->GetValue().Rotation, Fraction, 1.f);

		SetActorLocation(InterpLocation);
		SetActorRotation(InterpRotation);
	}
}

