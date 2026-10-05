#pragma once

#include "CoreMinimal.h"
#include "Settings/FGUserSettingCategory.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SETunerTypes.generated.h"

UCLASS()
class USETunerCategory : public UFGUserSettingCategory
{
	GENERATED_BODY()
public:
	USETunerCategory();
};

UCLASS()
class USETunerSubCategory : public UFGUserSettingCategory
{
	GENERATED_BODY()
public:
	USETunerSubCategory();
};

/** Registers one "cost multiplier" session setting per Space Elevator phase, shown when creating a world. */
UCLASS()
class USETunerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
};
