#include "Player/VaultingComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"

UVaultingComponent::UVaultingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	
}

void UVaultingComponent::Vault()
{
	Progress = 0.0f;
	StartingPosition = OwningCharacter->GetActorLocation();
	VaultState = EVaultState::EVS_Vaulting;
	
}

void UVaultingComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<ACharacter>(GetOwner());

	auto* Owning = GetOwner()->GetInstigator()->GetController();
	auto* Controller = Cast<APlayerController>(Owning);
	VaultWidget = CreateWidget(Controller, VaultWidgetClass);
}

void UVaultingComponent::VaultTick(const float DeltaSeconds)
{
	const float ProgressValue = Progress + DeltaSeconds / VaultSpeed;
	Progress = FMath::Clamp(ProgressValue, 0.0f, 1.0f);
	const FVector NewLocation = FMath::Lerp(StartingPosition, EndPosition, Progress);
	OwningCharacter->SetActorLocation(NewLocation);

	if (Progress >= 1.0f)
	{
		VaultState = EVaultState::EVS_NotVault;
	}
	
}
