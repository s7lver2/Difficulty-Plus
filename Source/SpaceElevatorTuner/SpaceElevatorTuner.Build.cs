using UnrealBuildTool;

public class SpaceElevatorTuner : ModuleRules
{
	public SpaceElevatorTuner(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = false;

		PublicDependencyModuleNames.AddRange(new[] {
			"Core", "CoreUObject", "Engine", "Projects",
			"FactoryGame", "SML"
		});
	}
}
