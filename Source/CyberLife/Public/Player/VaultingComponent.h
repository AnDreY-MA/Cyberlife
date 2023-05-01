#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VaultingComponent.generated.h"

UENUM(BlueprintType)
enum class EVaultState : uint8
{
	EVS_NotVault UMETA(DisplayName = "NotVaulting"),
	EVS_Vaulting UMETA(DisplayName = "Vaulting")
};

class ACharacter;
class UUserWidget;


UCLASS( ClassGroup=(Custom), Blueprintable )
class CYBERLIFE_API UVaultingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVaultingComponent();

	UFUNCTION(BlueprintCallable)
	void Vault();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	ACharacter* OwningCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Position", meta=(AllowPrivateAccess))
	FVector StartingPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Position", meta=(AllowPrivateAccess))
	FVector EndPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	float MinVaultingHeight { 70.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	float MaxVaultingHeight { 150.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	float Progress { 0.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	float VaultSpeed { 0.5f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	EVaultState VaultState { EVaultState::EVS_NotVault };

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess))
	TSubclassOf<UUserWidget> VaultWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UUserWidget* VaultWidget;

	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, meta=(BlueprintProtected))
	void VaultTick(float DeltaSeconds);
		
};
