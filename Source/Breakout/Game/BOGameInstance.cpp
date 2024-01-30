// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BOGameInstance.h"

void UBOGameInstance::ConnectToServer()
{
	m_Socket = ClientSocket::GetSingleton();
	m_Socket->InitSocket();

	connect = m_Socket->Connect("127.0.0.1", 12345);
	if (connect)
	{
		m_Socket->StartListen();
		UE_LOG(LogClass, Warning, TEXT("IOCP Server connect success!"));
		FString id = "testuser";
		FString pw = "1234";
		m_Socket->Send_Login_Info(TCHAR_TO_UTF8(*id), TCHAR_TO_UTF8(*pw));
	}
	else
	{
		UE_LOG(LogClass, Warning, TEXT("IOCP Server connect FAIL!"));
	}
}
