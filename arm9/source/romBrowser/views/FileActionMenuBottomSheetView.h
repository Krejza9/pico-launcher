#pragma once
#include "BottomSheetView.h"
#include "ChipView.h"
#include "gui/views/Label2DView.h"
#include "gui/FocusManager.h"

class IRomBrowserController;
class IFontRepository;

class FileActionMenuBottomSheetView : public BottomSheetView
{
public:
    FileActionMenuBottomSheetView(
        IRomBrowserController* romBrowserController,
        const MaterialColorScheme* materialColorScheme,
        const IFontRepository* fontRepository,
        const char* fileName);

    void SetGraphics(const ChipView::VramToken& chipVramToken)
    {
        _deleteChip.SetGraphics(chipVramToken);
    }

    void InitVram(const VramContext& vramContext) override;

    void Update() override;
    void Draw(GraphicsContext& graphicsContext) override;

    void Focus(FocusManager& focusManager) override
    {
        focusManager.Focus(&_deleteChip);
    }

    View* MoveFocus(View* currentFocus, FocusMoveDirection direction, View* source) override;

    bool HandleInput(const InputProvider& inputProvider, FocusManager& focusManager) override;

private:
    static constexpr u32 LABEL_WIDTH = 224;
    static constexpr u32 LABEL_MAX_LEN = 80;
    static constexpr u32 SCROLL_SPEED_FRAMES = 6;
    static constexpr u32 SCROLL_PAUSE_FRAMES = 60;
    static constexpr u32 SCROLL_SEPARATOR_LEN = 5;

    IRomBrowserController* _romBrowserController;
    const MaterialColorScheme* _materialColorScheme;
    Label2DView _titleLabel;
    ChipView _deleteChip;

    char16_t _scrollBuffer[LABEL_MAX_LEN * 2 + SCROLL_SEPARATOR_LEN + 1];
    u32 _scrollBufferLen;
    u32 _scrollOffset;
    u32 _scrollFrameCounter;
    bool _needsScroll;
    bool _scrollPaused;
    bool _chipSelected;
};
