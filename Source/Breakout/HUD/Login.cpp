// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/Login.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
#include "Sound/SoundCue.h"
void ULogin::NativeConstruct()
{
	Super::NativeConstruct();

	if (Login)
		Login->OnClicked.AddDynamic(this, &ULogin::PressLogin);
	if (Login)
		SignUp->OnClicked.AddDynamic(this, &ULogin::PressSignUp);

}

void ULogin::PressLogin()
{
	
	FString IDToString = ID->GetText().ToString();
	FString PasswordToString = Password->GetText().ToString();

	if(Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Login_Info(TCHAR_TO_UTF8(*IDToString), TCHAR_TO_UTF8(*PasswordToString));

	if (ClickSound)
	{
		PlaySound(ClickSound);
	}
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket->bLoginConnect)
	{
		RemoveFromParent();
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->bLoginConnect = false;
	}
}

void ULogin::PressSignUp()
{
	FString IDToString = ID->GetText().ToString();
	FString PasswordToString = Password->GetText().ToString();
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Account_PACKET(TCHAR_TO_UTF8(*IDToString), TCHAR_TO_UTF8(*PasswordToString));
}

