using UnrealBuildTool;

// Игровой модуль: классы шаблона Third Person, кошелёк и каталог магазина, настройки игры. Про UI он не знает.
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
			"EnhancedInput",      // шаблонные персонаж и контроллер
			"DeveloperSettings"   // URelayShopSettings в Project Settings
		});
	}
}
