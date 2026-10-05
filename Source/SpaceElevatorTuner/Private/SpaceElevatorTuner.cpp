#include "SpaceElevatorTuner.h"

#include "SETunerTypes.h"
#include "Engine/World.h"
#include "FGGameMode.h"
#include "FGGamePhase.h"
#include "ItemAmount.h"
#include "Resources/FGItemDescriptor.h"
#include "Registry/SessionSettingsRegistry.h"
#include "SessionSettings/SessionSetting.h"
#include "SessionSettings/SessionSettingsManager.h"
#include "Settings/FGUserSetting.h"
#include "Settings/FGUserSettingApplyType.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC( LogSpaceElevatorTuner, Log, All );

namespace SETuner
{
	/** Selectable multipliers, in percent. 100 = vanilla. */
	const TArray< int32 > Percents = { 50, 100, 200, 300, 500, 1000 };
	constexpr int32 DefaultPercent = 100;

	FString SettingId( int32 phaseNumber )
	{
		return FString::Printf( TEXT( "SpaceElevatorTuner.Phase%dCostPercentInt" ), phaseNumber );
	}

	// Data assets outlive worlds, so keep the vanilla costs and always scale from them.
	TMap< TWeakObjectPtr< UFGGamePhase >, TArray< FItemAmount > > Vanilla;

	const TArray< FItemAmount >& GetVanilla( UFGGamePhase* phase )
	{
		TArray< FItemAmount >& costs = Vanilla.FindOrAdd( phase );
		if( costs.IsEmpty() )
		{
			costs = phase->mCosts;
		}
		return costs;
	}

	/** Phases that actually cost items, in game order. Phase N of the settings is element N-1. */
	TArray< UFGGamePhase* > GetCostPhases()
	{
		TArray< UFGGamePhase* > result;
		for( UFGGamePhase* phase : UFGGamePhase::GetAllGamePhaseAssetsSorted() )
		{
			if( phase && !GetVanilla( phase ).IsEmpty() )
			{
				result.Add( phase );
			}
		}
		return result;
	}

	int32 Scale( int32 amount, int32 percent )
	{
		return FMath::Max( 1, FMath::CeilToInt( amount * ( percent / 100.0 ) ) );
	}

	FString MultiplierLabel( int32 percent )
	{
		return FString::Printf( TEXT( "x%s" ), *FString::SanitizeFloat( percent / 100.0 ) );
	}

	void ApplyForWorld( UWorld* world )
	{
		USessionSettingsManager* manager = world ? world->GetSubsystem< USessionSettingsManager >() : nullptr;
		if( !manager )
		{
			return;
		}

		const TArray< UFGGamePhase* > phases = GetCostPhases();
		for( int32 i = 0; i < phases.Num(); ++i )
		{
			int32 percent = manager->GetIntOptionValue( SettingId( i + 1 ) );
			if( percent <= 0 )
			{
				percent = DefaultPercent;
			}

			UFGGamePhase* phase = phases[ i ];
			phase->mCosts = GetVanilla( phase );
			for( FItemAmount& cost : phase->mCosts )
			{
				cost.Amount = Scale( cost.Amount, percent );
			}
			UE_LOG( LogSpaceElevatorTuner, Log, TEXT( "%s: cost %s" ), *phase->GetName(), *MultiplierLabel( percent ) );
		}
	}
}

USETunerCategory::USETunerCategory()
{
	mDisplayName = FText::FromString( TEXT( "Difficulty+" ) );
	mMenuPriority = 0.f;
}

USETunerSubCategory::USETunerSubCategory()
{
	mDisplayName = FText::FromString( TEXT( "Phase costs" ) );
	mMenuPriority = 0.f;
}

void USETunerSubsystem::Initialize( FSubsystemCollectionBase& collection )
{
	Super::Initialize( collection );
	collection.InitializeDependency< USMLSessionSettingsRegistry >();
	USMLSessionSettingsRegistry* registry = GetGameInstance()->GetSubsystem< USMLSessionSettingsRegistry >();
	if( !registry )
	{
		return;
	}

	const TArray< UFGGamePhase* > phases = SETuner::GetCostPhases();
	for( int32 i = 0; i < phases.Num(); ++i )
	{
		const TArray< FItemAmount >& vanilla = SETuner::GetVanilla( phases[ i ] );

		// One dropdown entry per multiplier, with the resulting item counts spelled out as the preview.
		auto* selector = NewObject< UFGUserSetting_IntSelector >( this );
		selector->DefaultValue = SETuner::DefaultPercent;
		selector->ShowAsDropdown = true;
		for( int32 percent : SETuner::Percents )
		{
			TArray< FString > parts;
			for( const FItemAmount& cost : vanilla )
			{
				parts.Add( FString::Printf( TEXT( "%s %d" ), *UFGItemDescriptor::GetItemName( cost.ItemClass ).ToString(),
											SETuner::Scale( cost.Amount, percent ) ) );
			}
			FIntegerSelection entry;
			entry.Value = percent;
			entry.Name = FText::FromString( FString::Printf( TEXT( "%s: %s" ), *SETuner::MultiplierLabel( percent ), *FString::Join( parts, TEXT( ", " ) ) ) );
			selector->IntegerSelectionValues.Add( entry );
		}

		auto* setting = NewObject< USMLSessionSetting >( this );
		setting->StrId = SETuner::SettingId( i + 1 );
		setting->DisplayName = FText::FromString( FString::Printf( TEXT( "Phase %d cost" ), i + 1 ) );
		setting->ToolTip = FText::FromString( FString::Printf(
			TEXT( "Multiplier for the items needed in Space Elevator phase %d. Each option lists the resulting cost. Can only be set when creating the world." ), i + 1 ) );
		setting->ApplyType = UFGUserSettingApplyType::StaticClass();
		setting->ValueSelector = selector;
		// Hide it once the game is running: the value is fixed at world creation.
		setting->VisibilityDisqualifiers = static_cast< int64 >( ESettingVisiblityDisqualifier::USAD_NotInGame );

		FSettingsWidgetLocationDescriptor location;
		location.CategoryClass = USETunerCategory::StaticClass();
		location.SubCategoryClass = USETunerSubCategory::StaticClass();
		location.MenuPriority = static_cast< float >( i );
		setting->WidgetsToCreate.Add( location );

		registry->RegisterSessionSetting( TEXT( "SpaceElevatorTuner" ), setting );
	}
}

void FSpaceElevatorTunerModule::StartupModule()
{
	GameModeHandle = FGameModeEvents::GameModeInitializedEvent.AddRaw( this, &FSpaceElevatorTunerModule::OnGameModeInitialized );
}

void FSpaceElevatorTunerModule::ShutdownModule()
{
	FGameModeEvents::GameModeInitializedEvent.Remove( GameModeHandle );
}

void FSpaceElevatorTunerModule::OnGameModeInitialized( AGameModeBase* gameMode )
{
	UWorld* world = gameMode ? gameMode->GetWorld() : nullptr;
	if( !world || !world->IsGameWorld() )
	{
		return;
	}

	// SML loads the saved session settings in its own GameModeInitialized handler, which is registered after ours.
	// Wait one tick so the values are in place before we read them.
	world->GetTimerManager().SetTimerForNextTick( [ weak = TWeakObjectPtr< UWorld >( world ) ]()
	{
		SETuner::ApplyForWorld( weak.Get() );
	} );
}

IMPLEMENT_MODULE( FSpaceElevatorTunerModule, SpaceElevatorTuner )
