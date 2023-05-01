// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Items/MeeleWeapon.h"
#include "Items/Weapon.h"
#include "Logging/LogMacros.h"
#include "Player/InteractionComponent.h"
#include "UI/InventoryWidget.h"

DEFINE_LOG_CATEGORY_STATIC(PlayerLog, All, All);

// Sets default values
APlayerCharacter::APlayerCharacter():
	CrouchEyeOffset(0.0f),
	CrouchSpeed(12.0f),
	MaxHealth(100.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComponent->SetupAttachment(GetRootComponent());
	CameraComponent->bUsePawnControlRotation = false; 

	MeshArms = CreateDefaultSubobject<USkeletalMeshComponent>("Arms");
	MeshArms->SetupAttachment(CameraComponent);
	
	DefaultGrabObjectLocation = CreateDefaultSubobject<UArrowComponent>("DefaultLocationObject");
	DefaultGrabObjectLocation->SetupAttachment(CameraComponent);

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>("Handle");
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("InteractionComponent");

	CurrentHealth = MaxHealth;
	
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const auto* Contr= Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Contr->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
		
		ActiveInventoryWidget = Cast<UInventoryWidget>(CreateWidget(GetWorld(), InventoryWidgetClass));
		ActiveInventoryWidget->Init(InteractionComponent->GetInventory(), 50.0f);
	}

	InteractionComponent->Init(PhysicsHandle, DefaultGrabObjectLocation, CameraComponent);
	InteractionComponent->OnWeaponEquip.AddDynamic(this, &APlayerCharacter::EquipWeapon);
	InteractionComponent->OnWeaponUnEquip.AddDynamic(this, &APlayerCharacter::UnEquipWeapon);
	
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float CrouchInterpTime = FMath::Min(1.0f, CrouchSpeed * DeltaSeconds);
	CrouchEyeOffset = (1.0f - CrouchInterpTime) * CrouchEyeOffset;
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &APlayerCharacter::StartCrouch);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		//Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Interact);

		//Open Inventory
		EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ToggleInventoryWidget);

		//Attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Attack);
		
	}

}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::Jumping()
{
	Jump();

}

void APlayerCharacter::Interact() 
{
	InteractionComponent->Interact();
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if(HalfHeightAdjust == 0) return;

	const float StartBaseEyeHeight = ScaledHalfHeightAdjust;
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CrouchEyeOffset.Z += StartBaseEyeHeight - BaseEyeHeight + HalfHeightAdjust;
	CameraComponent->SetRelativeLocation(FVector{0.0f, 0.0f, BaseEyeHeight}, false);
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if(HalfHeightAdjust == 0) return;

	const float StartBaseEyeHeight = ScaledHalfHeightAdjust;
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CrouchEyeOffset.Z += StartBaseEyeHeight - BaseEyeHeight - HalfHeightAdjust;
	CameraComponent->SetRelativeLocation(FVector{0.0f, 0.0f, BaseEyeHeight}, false);
}

void APlayerCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	CameraComponent->GetCameraView(DeltaTime, OutResult);
	OutResult.Location += CrouchEyeOffset;
	
}

void APlayerCharacter::StartCrouch()
{
	if(bIsCrouched == false)
	{
		Crouch();
	}
	else if(bIsCrouched == true)
	{
		UnCrouch();
	}
}

void APlayerCharacter::ToggleInventoryWidget()
{
	auto* PlayerController= Cast<APlayerController>(Controller);
	
	if(ActiveInventoryWidget->IsInViewport())
	{
		ActiveInventoryWidget->RemoveFromParent();
		PlayerController->SetShowMouseCursor(false);
		const FInputModeGameOnly InputModeGameOnly;
		PlayerController->SetInputMode(InputModeGameOnly);
	}
	else
	{
		ActiveInventoryWidget->AddToViewport();
		FInputModeGameAndUI InputModeGameAndUI;
		InputModeGameAndUI.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputModeGameAndUI);
		PlayerController->SetShowMouseCursor(true);
	}
}

void APlayerCharacter::EquipWeapon(AWeapon* Weapon)
{
	const USkeletalMeshSocket* HandSocket = MeshArms->GetSocketByName(FName("WeaponSocket"));
	if (!HandSocket) return;
	
	HandSocket->AttachActor(Weapon, MeshArms);
	OnEquipWeapon.Broadcast();
	if (Cast<AMeeleWeapon>(Weapon)) OnEquipMeeleWeapon.Broadcast();
	
}

void APlayerCharacter::UnEquipWeapon()
{
	OnUnEquipWeapon.Broadcast();
}

void APlayerCharacter::Attack()
{
	if (InteractionComponent->IsHoldingObject())
	{
		InteractionComponent->ThrowObject();
	}
	else
	{
		InteractionComponent->AttackWeapon();
		const auto* MeeleWeapon = Cast<AMeeleWeapon>(InteractionComponent->GetWaepon());
		if (MeeleWeapon && MeeleAttackAnim)
		{
			UAnimInstance* AnimInstance = MeshArms->GetAnimInstance();
			check(AnimInstance);
			AnimInstance->Montage_Play(MeeleAttackAnim);
		}
	}
	
}

void APlayerCharacter::ApplyDamageMomentum(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator,
	AActor* DamageCauser)
{
	Super::ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);

	CurrentHealth -= DamageTaken;

	if(OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(CurrentHealth);
	}
	
	if (CurrentHealth <= 0.0)
	{
		UE_LOG(PlayerLog, Display, TEXT("Dead"));
	}	
}
