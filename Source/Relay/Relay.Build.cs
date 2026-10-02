using UnrealBuildTool;

// Игровой модуль: классы шаблона Third Person. Про UI он не знает.
public class Relay : ModuleRules
{
	public Relay(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore",
			"EnhancedInput"       // шаблонные персонаж и контроллер
		});
	}
}
