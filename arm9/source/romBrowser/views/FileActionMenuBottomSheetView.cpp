#include "common.h"
#include "core/StringUtil.h"
#include "gui/font/nitroFont2.h"
#include "gui/GraphicsContext.h"
#include "gui/input/InputProvider.h"
#include "themes/material/MaterialColorScheme.h"
#include "../IRomBrowserController.h"
#include "FileActionMenuBottomSheetView.h"

FileActionMenuBottomSheetView::FileActionMenuBottomSheetView(
    IRomBrowserController* romBrowserController, const MaterialColorScheme* materialColorScheme,
    const IFontRepository* fontRepository, const char* fileName)
    : _romBrowserController(romBrowserController)
    , _materialColorScheme(materialColorScheme)
    , _titleLabel(LABEL_WIDTH, 16, LABEL_MAX_LEN * 2 + SCROLL_SEPARATOR_LEN,
        fontRepository->GetFont(FontType::Regular10))
    , _deleteChip(md::sys::color::surfaceContainerLow, materialColorScheme, fontRepository)
    , _scrollBufferLen(0)
    , _scrollOffset(0)
    , _scrollFrameCounter(0)
    , _needsScroll(false)
    , _scrollPaused(true)
    , _chipSelected(false)
{
    // Convert to char16_t and measure
    char16_t fileNameU16[LABEL_MAX_LEN + 1];
    u32 nameLen = StringUtil::Copy(fileNameU16, fileName, LABEL_MAX_LEN + 1);

    u32 textWidth, textHeight;
    nft2_measureString(fontRepository->GetFont(FontType::Regular10), fileNameU16, textWidth, textHeight);

    if (textWidth <= LABEL_WIDTH)
    {
        StringUtil::Copy(_scrollBuffer, fileNameU16, LABEL_MAX_LEN + 1);
    }
    else
    {
        // Build scroll buffer: "filename     filename     "
        static const char16_t separator[] = u"     ";
        u32 pos = 0;
        for (u32 i = 0; i < nameLen && pos < LABEL_MAX_LEN * 2 + SCROLL_SEPARATOR_LEN; i++)
            _scrollBuffer[pos++] = fileNameU16[i];
        for (u32 i = 0; i < SCROLL_SEPARATOR_LEN && pos < LABEL_MAX_LEN * 2 + SCROLL_SEPARATOR_LEN; i++)
            _scrollBuffer[pos++] = separator[i];
        for (u32 i = 0; i < nameLen && pos < LABEL_MAX_LEN * 2 + SCROLL_SEPARATOR_LEN; i++)
            _scrollBuffer[pos++] = fileNameU16[i];
        _scrollBuffer[pos] = 0;
        _scrollBufferLen = nameLen + SCROLL_SEPARATOR_LEN;
        _needsScroll = true;
    }

    _titleLabel.SetText(_scrollBuffer);
    AddChildTail(&_titleLabel);
    _deleteChip.SetText(u"Delete");
    _deleteChip.SetSelected(false);
    AddChildTail(&_deleteChip);
}

void FileActionMenuBottomSheetView::Update()
{
    BottomSheetView::Update();
    _titleLabel.SetPosition(16, _position.y + 16);
    _deleteChip.SetPosition(16, _position.y + 40);

    if (_needsScroll)
    {
        _scrollFrameCounter++;
        if (_scrollPaused)
        {
            if (_scrollFrameCounter >= SCROLL_PAUSE_FRAMES)
            {
                _scrollPaused = false;
                _scrollFrameCounter = 0;
            }
        }
        else
        {
            if (_scrollFrameCounter >= SCROLL_SPEED_FRAMES)
            {
                _scrollFrameCounter = 0;
                _scrollOffset++;
                if (_scrollOffset >= _scrollBufferLen)
                {
                    _scrollOffset = 0;
                    _scrollPaused = true;
                }
                _titleLabel.SetText(_scrollBuffer + _scrollOffset);
            }
        }
    }
}

void FileActionMenuBottomSheetView::Draw(GraphicsContext& graphicsContext)
{
    graphicsContext.SetClipArea(GetBounds());
    u32 oldPrio = graphicsContext.SetPriority(1);
    {
        _titleLabel.SetBackgroundColor(_materialColorScheme->GetColor(md::sys::color::surfaceContainerLow));
        _titleLabel.SetForegroundColor(_materialColorScheme->onSurface);
        BottomSheetView::Draw(graphicsContext);
    }
    graphicsContext.SetPriority(oldPrio);
    graphicsContext.ResetClipArea();
}

View* FileActionMenuBottomSheetView::MoveFocus(View* currentFocus,
    FocusMoveDirection direction, View* source)
{
    _chipSelected = !_chipSelected;
    _deleteChip.SetSelected(_chipSelected);
    return &_deleteChip;

}

bool FileActionMenuBottomSheetView::HandleTouch(const Point& touchPosition, FocusManager& focusManager)
{
    if (_deleteChip.GetBounds().Contains(touchPosition))
    {
        focusManager.Focus(&_deleteChip);
        if (!_chipSelected)
        {
            // First tap selects the chip
            _chipSelected = true;
            _deleteChip.SetSelected(true);
        }
        else
        {
            // Second tap activates delete
            _romBrowserController->DeleteSelectedFile();
        }
        return true;
    }
    return false;
}

bool FileActionMenuBottomSheetView::HandleInput(const InputProvider& inputProvider, FocusManager& focusManager)
{
    if (inputProvider.Triggered(InputKey::B))
    {
        _romBrowserController->HideFileActionMenu();
        return true;
    }
    else if (inputProvider.Triggered(InputKey::A))
    {
        if (_chipSelected && focusManager.IsFocusInside(&_deleteChip))
        {
            _romBrowserController->DeleteSelectedFile();
            return true;
        }
    }
    return false;
}
