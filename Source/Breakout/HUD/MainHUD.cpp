// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MainHUD.h"
#include "GameFramework/PlayerController.h"
#include "HUD/SelectWeaponUi.h"
#include "HUD/CharacterUi.h"
void AMainHUD::BeginPlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	SelectWeapon = CreateWidget<USelectWeaponUi>(PlayerController, SelectWeaponClass);

	AddCharacterOverlay();
}

void AMainHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterUiClass)
	{
		CharacterUi = CreateWidget<UCharacterUi>(PlayerController, CharacterUiClass);
		CharacterUi->AddToViewport();
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

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairRight)
		{
			FVector2D Spread(+SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, Spread);
		}
		if (HUDPackage.CrosshairBottom)
		{
			FVector2D Spread(0.f, +SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, Spread);
		}

	}

}

void AMainHUD::AddSelectWeapon()
{
	SelectWeapon->AddToViewport();
}

void AMainHUD::RemoveSelectWeapon()
{
	SelectWeapon->RemoveFromViewport();
}
