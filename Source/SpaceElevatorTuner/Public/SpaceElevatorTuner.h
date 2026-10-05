#pragma once

#include "Modules/ModuleManager.h"

class AGameModeBase;

class FSpaceElevatorTunerModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void OnGameModeInitialized( AGameModeBase* gameMode );

	FDelegateHandle GameModeHandle;
};
