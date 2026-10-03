#pragma once

#include "CoreGlobals.h"
#include "Framework/Application/IInputProcessor.h"
#include "Input/Events.h"

// Отличает настоящее движение мыши от синтетического. Slate каждый тик пересылает виджетам «движение»
// неподвижного курсора, а при смене устройства подменяет курсор, не двигая его. Тогда кнопка под стоящей
// мышью получает наведение, хотя пользователь работает клавиатурой или геймпадом.
// Предпроцессоры ввода видят только настоящие события ОС и видят их раньше виджетов.

/** Input preprocessor that records the frame of the last real (not synthetic) mouse move. */
class FRelayPointerMotionTracker : public IInputProcessor
{
public:
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		if (!MouseEvent.IsTouchEvent())
		{
			LastMoveFrame = GFrameCounter;
			LastMoveUserIndex = MouseEvent.GetUserIndex();
		}
		return false;
	}

	virtual const TCHAR* GetDebugName() const override { return TEXT("RelayPointerMotionTracker"); }

	bool HasMovedThisFrame(int32 UserIndex) const
	{
		return LastMoveFrame == GFrameCounter && LastMoveUserIndex == UserIndex;
	}

private:
	uint64 LastMoveFrame = MAX_uint64;
	int32 LastMoveUserIndex = INDEX_NONE;
};
