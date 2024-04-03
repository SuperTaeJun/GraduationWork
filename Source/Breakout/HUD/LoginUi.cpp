// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/LoginUi.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"

ULoginUi::ULoginUi()
{
	if(Login)
		Login->OnClicked.AddDynamic(this, &ULoginUi::PressLogin);
}

void ULoginUi::PressLogin()
{
}
