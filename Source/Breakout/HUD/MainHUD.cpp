// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MainHUD.h"
#include "GameFramework/PlayerController.h"
#include "HUD/SelectWeaponUi.h"
#include "HUD/RespawnSelect.h"
#include "HUD/CharacterUi.h"
#include "HUD/MatchingUi.h"
#include "HUD/EscapeToolNumUi.h"

void AMainHUD::BeginPlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	CharacterUi = CreateWidget<UCharacterUi>(PlayerController, CharacterUiClass);
	EscapeToolNumUi = CreateWidget<UEscapeToolNumUi>(PlayerController, EscapeToolNumUiClass);
	SelectWeapon = CreateWidget<USelectWeaponUi>(PlayerController, SelectWeaponClass);
	AddCharacterOverlay();
	CharacterUi->SetVisibility(ESlateVisibility::Collapsed);
	AddToolNumUi();
	EscapeToolNumUi->SetVisibility(ESlateVisibility::Collapsed);
}

void AMainHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();

	if (CharacterUiClass)
	{
		CharacterUi->AddToViewport();
	}
}

void AMainHUD::RemoveCharacterOverlay()
{
	if (CharacterUi)
		CharacterUi->SetVisibility(ESlateVisibility::Collapsed);
	//CharacterUi->RemoveFromParent();
}

void AMainHUD::StartSetVisibility()
{
	if (CharacterUi && EscapeToolNumUi)
	{
		CharacterUi->SetVisibility(ESlateVisibility::Visible);
		EscapeToolNumUi->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, FLinearColor::White);
}

void AMainHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		if (HUDPackage.CrosshairCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairLeft)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairRight)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairTop)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairBottom)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, Spread);
		}

	}

}

void AMainHUD::AddToolNumUi()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController)
	{
		if (EscapeToolNumUi)
			EscapeToolNumUi->AddToViewport();
	}
}

void AMainHUD::RemoveToolNumUi()
{
	if (EscapeToolNumUi)
	{
		//EscapeToolNumUi->RemoveFromParent();
		EscapeToolNumUi->SetVisibility(ESlateVisibility::Collapsed);
		//EscapeToolNumUi->
	}
}

void AMainHUD::AddSelectWeapon()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController)
	{
		SelectWeapon = CreateWidget<USelectWeaponUi>(PlayerController, SelectWeaponClass);
		if (SelectWeapon)
			SelectWeapon->AddToViewport();
	}

}

void AMainHUD::AddSelectRespawn()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController)
	{
		RespawnSelectUi = CreateWidget<URespawnSelect>(PlayerController, RespawnSelectUiClass);
		if (RespawnSelectUi)
			RespawnSelectUi->AddToViewport();
	}

}

void AMainHUD::AddMatchingUi()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController)
	{
		MatchingUi = CreateWidget<UMatchingUi>(PlayerController, MatchingUiClass);
		if (MatchingUi)
			MatchingUi->AddToViewport();
	}
}

void AMainHUD::RemoveSelectWeapon()
{
	SelectWeapon->RemoveFromParent();
}

void AMainHUD::RemoveRespawnSelect()
{
	RespawnSelectUi->RemoveFromParent();
}

void AMainHUD::RemoveMatchingUi()
{
	MatchingUi->RemoveFromParent();
}
