// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/BulletHole.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
ABulletHole::ABulletHole()
{
}

void ABulletHole::OnRebuildGeneratedMesh(UDynamicMesh* TargetMesh)
{
	Super::OnRebuildGeneratedMesh(TargetMesh);
	
	FGeometryScriptPrimitiveOptions Options;
	FTransform Transform;
	BoxDimensionX = 400.f;
	BoxDimensionY = 40.f;
	BoxDimensionZ = 400.f;
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(TargetMesh, Options, Transform, BoxDimensionX, BoxDimensionY, BoxDimensionZ);


}

void ABulletHole::ExecuteRebuildGeneratedMeshIfPending()
{
	Super::ExecuteRebuildGeneratedMeshIfPending();
}

void ABulletHole::BeiginPlay()
{
	Super::BeginPlay();
}
