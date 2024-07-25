// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/Login.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
#include "Sound/SoundCue.h"
void ULogin::NativeConstruct()
{
	Super::NativeConstruct();

	if (Fail)
	{
		Fail->SetVisibility(ESlateVisibility::Collapsed);
		Fail->OnClicked.AddDynamic(this, &ULogin::PressFail);
	}

	if (Login)
	{
		Login->OnClicked.AddDynamic(this, &ULogin::PressLogin);
		Login->OnHovered.AddDynamic(this, &ULogin::HoverLog);
	}
	if (SignUp)
		SignUp->OnClicked.AddDynamic(this, &ULogin::PressSignUp);

}
void ULogin::HoverLog()
{
	FString IDToString = ID->GetText().ToString();
	FString PasswordToString = Password->GetText().ToString();
	FString IDToIP = IP->GetText().ToString();

	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Login_Info(TCHAR_TO_UTF8(*IDToString), TCHAR_TO_UTF8(*PasswordToString));

}
void ULogin::PressLogin()
{

	//로그인실패하면
	if (Fail)
	{
		Fail->SetVisibility(ESlateVisibility::Visible);
	}
	///////////////////////////////////////////////}	
	// 
	//로그인 성공하면
	//RemoveFromParent();
	////////////////////////////////


	if (ClickSound)
	{
		PlaySound(ClickSound);
	}
}

void ULogin::PressSignUp()
{
	FString IDToString = ID->GetText().ToString();
	FString PasswordToString = Password->GetText().ToString();
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Account_PACKET(TCHAR_TO_UTF8(*IDToString), TCHAR_TO_UTF8(*PasswordToString));
}

void ULogin::PressFail()
{
	Fail->SetVisibility(ESlateVisibility::Collapsed);
}