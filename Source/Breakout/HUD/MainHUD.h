#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainHUD.generated.h"

USTRUCT(BlueprintType)
struct FCrosshairPackage
{
	GENERATED_BODY()
	public:
	class UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	float CrosshairSpread;
};

UCLASS()
class BREAKOUT_API AMainHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Player State")
	TSubclassOf<class UUserWidget>SelectWeaponClass;
	TObjectPtr<class USelectWeaponUi> SelectWeapon;

	UPROPERTY(EditAnywhere, Category = "Player State")
	TSubclassOf<class UUserWidget>CharacterUiClass;
	TObjectPtr<class UCharacterUi> CharacterUi;


	virtual void DrawHUD() override;

	void AddSelectWeapon();
	void RemoveSelectWeapon();
protected:
	virtual void BeginPlay() override;
	void AddCharacterOverlay();

private:
	FCrosshairPackage HUDPackage;
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread);
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
public:
	FORCEINLINE void SetHUDPackage(const FCrosshairPackage& Package) { HUDPackage = Package; }
};
