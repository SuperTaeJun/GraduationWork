// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BOGameMode.h"
#include "Character/CharacterBase.h"
#include "Game/BOGameInstance.h"
#include "Character/CharacterBase.h"
ABOGameMode::ABOGameMode()
{
	/*m_Socket = ClientSocket::GetSingleton();
	m_Socket->InitSocket();

	connect = m_Socket->Connect("127.0.0.1", 12345);
	if (connect)
	{
		m_Socket->StartListen();
		UE_LOG(LogClass, Log, TEXT("IOCP Server connect success!"));
	}
	else
	{
		UE_LOG(LogClass, Warning, TEXT("IOCP Server connect FAIL!"));
	}*/
}

void ABOGameMode::PlayerRemove(ACharacterBase* RemovedCharacter, ACharacterController* RemovedCharacterController, ACharacterController* AttackerController)
{
	if (RemovedCharacter)
	{
		RemovedCharacter->Dead();
	}
}

//void ABOGameMode::BeginPlay()
//{
//
//}

