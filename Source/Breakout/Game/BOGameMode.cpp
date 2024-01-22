// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BOGameMode.h"
#include "Character/CharacterBase.h"
ABOGameMode::ABOGameMode()
{
	Socket = ClientSocket::GetSingleton();
	Socket->InitSocket();
	bIsConnected = Socket->Connect("127.0.0.1", 12345);
	if (bIsConnected) {
		UE_LOG(LogClass, Log, TEXT("IOCP Server connect success!"));
		Socket->SetGameMode(this);
	}
}

void ABOGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsConnected) return;

}

void ABOGameMode::BeginPlay()
{
	Super::BeginPlay();
	//UE_LOG(LogTemp, Error, TEXT("INITAILAIZING ERROR"));
	// Recv 스레드 시작
	Socket->StartListen();
}

void ABOGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//Socket->LogoutCharacter(SessionId);
	Socket->CloseSocket();
	Socket->StopListen();
}