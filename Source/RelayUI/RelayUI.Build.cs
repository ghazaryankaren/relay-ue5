using UnrealBuildTool;

// Весь UI проекта. Модуль ClientOnly: выделенный сервер его не собирает и не грузит.
// Зависимость от игрового модуля только в эту сторону: Relay про UI не знает.
public class RelayUI : ModuleRules
{
	public RelayUI(ReadOnlyTargetRules Target) : base(Target)
	{
		// Только то, что видно в публичных заголовках модуля
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG",                // UUserWidget
			"CommonUI",           // активируемые экраны и стеки слоёв
			"ModelViewViewModel", // базовый класс viewmodel
			"FieldNotification",  // FieldNotify-поля viewmodel
			"GameplayTags"        // теги слоёв и экранов в API роутера
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CommonInput",        // режимы ввода Menu / Game
			"DeveloperSettings",  // URelayUISettings в Project Settings
			"EnhancedInput",      // контекст IMC_UI и действия горячих клавиш экранов
			"Relay",              // кошелёк, каталог магазина и настройки игры
			"Slate",              // событие смены фокуса и предпроцессор ввода
			"SlateCore"
		});
	}
}
