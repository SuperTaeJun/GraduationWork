// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MatchingUi.h"
#include "Components/TextBlock.h"

void UMatchingUi::SetWaitingText()
{
	WaitingText->SetText(FText::FromString("Waiting for Other Player"));
}

void UMatchingUi::SetContingText(float Time)
{
	FString CountText = FString::Printf(TEXT("%d"), Time);
	WaitingText->SetText(FText::FromString("CountText"));
}
