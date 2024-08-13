// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/Login.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
#include "Sound/SoundCue.h"
#include "Components/TextBlock.h"

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
	if (!Cast<UBOGameInstance>(GetGameInstance())->m_Socket->bLoginConnect) {
		if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
			Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Login_Info(TCHAR_TO_UTF8(*IDToString), TCHAR_TO_UTF8(*PasswordToString));
	}
}
void ULogin::PressLogin()
{

	////로그인실패하면
	if (Fail && !Cast<UBOGameInstance>(GetGameInstance())->m_Socket->bLoginConnect)
	{
		if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket->failType == 0)
		{
			FString Text = "Overlap ID";
			FailText->SetText(FText::FromString(Text));
			Fail->SetVisibility(ESlateVisibility::Visible);
		}
		else if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket->failType == 1)
		{
			FString Text = "Wrong ID";
			FailText->SetText(FText::FromString(Text));
			Fail->SetVisibility(ESlateVisibility::Visible);
		}
		else if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket->failType == 2)
		{
			FString Text = "Wrong PW";
			FailText->SetText(FText::FromString(Text));
			Fail->SetVisibility(ESlateVisibility::Visible);
		}
	}

	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket->bLoginConnect)
		RemoveFromParent();



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