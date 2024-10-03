// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/EscapeTool.h"
#include "Character/CharacterBase.h"
#include "ProceduralMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
#include "HUD/ETPercentBar.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"

AEscapeTool::AEscapeTool()
{
	PercentBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("PercentBar"));
	PercentBar->SetupAttachment(RootComponent);

	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	bDetected = false;

	////MeshA = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/Maps/MapAsset/oilDrum_2.oilDrum_2")).Object;
	//ProcMeshUtillity->GetMeshDataFromStaticMesh(MeshA, DataA, 0, 0, false);
	//ProcMeshUtillity->UnifyTri(DataA);
	////MeshB = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/Maps/MapAsset/jem.jem")).Object;
	//ProcMeshUtillity->GetMeshDataFromStaticMesh(MeshB, DataB, 0, 0, false);
	//ProcMeshUtillity->UnifyTri(DataB);


	////부족하면 추가해주기
	//if (DataB.Verts.Num() > DataA.Verts.Num())
	//{
	//	LoopNum = DataB.Verts.Num() - DataA.Verts.Num();

	//	for (int i = 0; i < (LoopNum/3.0) ; ++i)
	//	{
	//		RandNum = UKismetMathLibrary::RandomIntegerInRange(0, DataA.Verts.Num()-1);
	//		MeshData TempMeshData = DataA;
	//		for (int j = 0; j < 3; ++j)
	//		{
	//			DataA.Tris.Add(TempMeshData.Verts.Num());
	//			DataA.Verts.Add(TempMeshData.Verts[RandNum]);
	//			DataA.Normals.Add(FVector(0.f, 0.f, 1.f));
	//			DataA.UVs.Add(FVector2D(0.f, 0.f));
	//			DataA.Colors.Add(FLinearColor::Black);
	//		}
	//	}
	//}

	//ProcMeshUtillity->SetColorData(DataA, FLinearColor::Black);
	//ProcMeshUtillity->SetColorData(DataB, FLinearColor::Black);

	//InterpData = DataA;

	//TArray<FProcMeshTangent> Temp{};
	//ProceduralMesh->CreateMeshSection_LinearColor
	//(
	//	0,
	//	InterpData.Verts,
	//	InterpData.Tris,
	//	InterpData.Normals,
	//	InterpData.UVs,
	//	InterpData.Colors,
	//	Temp,
	//	true
	//);
	//ProceduralMesh->SetMaterial(0, OldMaterial);
}

void AEscapeTool::BeginPlay()
{
	Super::BeginPlay();
	inst = Cast<UBOGameInstance>(GetGameInstance());

	//생성자가 아니라 BEGINPLAY에서 지연처리하면 잘 읽어와짐 
	SetProcMesh();

	DynamicMaterial = UMaterialInstanceDynamic::Create(OldMaterial, this);
	if (DynamicMaterial)
	{
		ProceduralMesh->SetMaterial(0, DynamicMaterial);
		DynamicMaterial->SetScalarParameterValue(FName("Alpha"), 0.f);
	}
	if (AreaSphere)
	{
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AEscapeTool::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AEscapeTool::OnSphereEndOverlap);
	}
	PercentBar->SetVisibility(false);

	UE_LOG(LogTemp, Warning, TEXT("DATA A : %d"), DataA.Verts.Num());
	UE_LOG(LogTemp, Warning, TEXT("DATA B : %d"), DataB.Verts.Num());
	UE_LOG(LogTemp, Warning, TEXT("DATA Interp : %d"), InterpData.Verts.Num());

	UpdatePercent(Cur);
}

void AEscapeTool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bOverlap == 2 && Cur > 0.f)
	{
		TransformMesh(DeltaTime, true, true);
		UpdatePercent(Cur);
	}
	else 	if (bOverlap == 1)
	{
		TransformMesh(DeltaTime, true, false);
		UpdatePercent(Cur);
	}


}

void AEscapeTool::Destroyed()
{
	Super::Destroyed();
	if (inst)
	{
		//inst->m_Socket->Send_Destroyed_item_packet(ItemID, inst->GetPlayerID());
		//inst->m_Socket->Send_item_Anim_packet(inst->GetPlayerID(), 1);
	}
}

void AEscapeTool::TransformMesh(float DeltaTime, bool Clamp, bool TransformReverse)
{

	if (OverlapedCharacter && bDetected)
	{
		if (Cur >= 1.f)
		{
			OverlapedCharacter->SetbCanObtainEscapeTool(true);
			PercentBar->SetVisibility(false);
			bOverlap = 0;
		}
		else if (Cur <= 0.f)
		{
			bOverlap = 0;
		}
	}

	Cur = FMath::Clamp(Time, 0.f, 1.f);

	DynamicMaterial->SetScalarParameterValue(FName("Alpha"), Cur);
	//InterpMeshData(InterpData, Data1, Data2, Cur, Clamp);
	ProcMeshUtillity->InterpMeshData(InterpData, DataA, DataB, Cur, Clamp);
	ProceduralMesh->UpdateMeshSection_LinearColor(0, InterpData.Verts, InterpData.Normals, InterpData.UVs, InterpData.Colors, TArray<FProcMeshTangent>());

	if (TransformReverse)
	{
		Time = Time - (DeltaTime * MorphingSpeed);
	}
	else
	{
		Time = Time + (DeltaTime * MorphingSpeed);
	}

	UpdatePercent(Cur);
}

void AEscapeTool::SetHideMesh()
{
	ProceduralMesh->SetHiddenInGame(true);
	Destroy();
}

void AEscapeTool::SetProcMesh()
{
	ProcMeshUtillity->GetMeshDataFromStaticMesh(MeshA, DataA);
	ProcMeshUtillity->UnifyTri(DataA);
	ProcMeshUtillity->GetMeshDataFromStaticMesh(MeshB, DataB);
	ProcMeshUtillity->UnifyTri(DataB);


	//부족하면 추가해주기
	if (DataB.Verts.Num() > DataA.Verts.Num())
	{
		LoopNum = DataB.Verts.Num() - DataA.Verts.Num();

		for (int i = 0; i < (LoopNum / 3.0); ++i)
		{
			RandNum = UKismetMathLibrary::RandomIntegerInRange(0, DataA.Verts.Num() - 1);
			MeshData TempMeshData = DataA;
			for (int j = 0; j < 3; ++j)
			{
				DataA.Tris.Add(TempMeshData.Verts.Num());
				DataA.Verts.Add(TempMeshData.Verts[RandNum]);
				TempMeshData.Verts.SetNum(DataA.Verts.Num());
				DataA.Normals.Add(FVector(0.f, 0.f, 1.f));
				DataA.UVs.Add(FVector2D(0.f, 0.f));
				DataA.Colors.Add(FLinearColor::Black);
			}
		}
	}

	ProcMeshUtillity->SetColorData(DataA, FLinearColor::Black);
	ProcMeshUtillity->SetColorData(DataB, FLinearColor::Black);

	InterpData = DataA;

	TArray<FProcMeshTangent> Temp{};
	ProceduralMesh->CreateMeshSection_LinearColor
	(
		0,
		InterpData.Verts,
		InterpData.Tris,
		InterpData.Normals,
		InterpData.UVs,
		InterpData.Colors,
		Temp,
		true
	);
	ProceduralMesh->SetMaterial(0, OldMaterial);
}

void AEscapeTool::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
		OverlapedCharacter = Cast<ACharacterBase>(OtherActor);
		if (OverlapedCharacter)
		{
			OverlapedCharacter->OverlappingEscapeTool = this;
		}
		if (bDetected)
		{
			PercentBar->SetVisibility(true);
		}

}

void AEscapeTool::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	ACharacterBase* characterbase = Cast<ACharacterBase>(OtherActor);

	if (characterbase)
	{
		PercentBar->SetVisibility(false);
		characterbase->SetbCanObtainEscapeTool(false);
		characterbase->OverlappingEscapeTool = nullptr;
		bOverlap = 2;
		if (inst && characterbase->GetMainController())
			inst->m_Socket->Send_Mopp_Sync_packet(ItemID, 2, inst->GetPlayerID());
	}
}

void AEscapeTool::UpdatePercent(float Percent)
{
	if (PercentBar->GetWidget())
	{
		Cast<UETPercentBar>(PercentBar->GetWidget())->PercentBar->SetPercent(Percent);
	}
}
