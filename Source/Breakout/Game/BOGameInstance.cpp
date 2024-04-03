// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BOGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CharacterController.h"


void UBOGameInstance::Init()
{
	Super::Init();
	//ACharacterController* ChController = Cast<ACharacterController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	m_Socket = ClientSocket::GetSingleton();
	
	connect = m_Socket->Connect();
	if (connect)
	{
		m_Socket->StartListen();
		m_Socket->SetGameInstance(this);
		//c_socket->StartListen();
		UE_LOG(LogClass, Warning, TEXT("IOCP Server connect success!"));
		
	}
	else UE_LOG(LogClass, Warning, TEXT("IOCP Server connect fail!"));
}

