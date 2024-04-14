// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/Login.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"

void ULogin::NativeConstruct()
{
	Super::NativeConstruct();

	if (Login)
		Login->OnClicked.AddDynamic(this, &ULogin::PressLogin);

}

void ULogin::PressLogin()
{
	//UE_LOG(LogTemp, Warning, TEXT("ID : %s"), ID->GetText().ToString());
	//UE_LOG(LogTemp, Warning, TEXT("Password : %s"), Password->GetText().ToString());

	FString IDToString = ID->GetText().ToString();
	FString PasswordToString = Password->GetText().ToString();

	if(Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Login_Info(TCHAR_TO_UTF8(*IDToString), TCHAR_TO_UTF8(*PasswordToString));

	RemoveFromParent();
}

