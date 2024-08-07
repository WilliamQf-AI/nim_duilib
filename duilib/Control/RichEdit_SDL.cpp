#include "RichEdit_SDL.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/WindowDropTarget.h"
#include "duilib/Core/ControlDropTarget.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/BitmapHelper_Windows.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Control/Menu.h"
#include "duilib/Box/VBox.h"
#include "duilib/Control/Button.h"

#ifdef _DEBUG
#include "duilib/Utils/FileUtil.h"
#endif

#ifdef DUILIB_BUILD_FOR_SDL
#include <SDL3/SDL.h>

namespace ui {

RichEdit::RichEdit(Window* pWindow) :
    ScrollBox(pWindow, new Layout),
    m_bWantTab(true),
    m_bWantReturn(false),
    m_bWantCtrlReturn(false),
    m_bAllowPrompt(false),
    m_bSelAllEver(false),         
    m_bNoSelOnKillFocus(true), 
    m_bSelAllOnFocus(false),    
    m_bNoCaretReadonly(false),
    m_bIsCaretVisiable(false),
    m_bIsComposition(false),
    m_iCaretPosX(0),
    m_iCaretPosY(0),
    m_iCaretWidth(0),
    m_iCaretHeight(0),
    m_nLineHeight(0),
    m_sFontId(),
    m_sTextColor(),
    m_sDisabledTextColor(),
    m_sPromptColor(),
    m_sPromptText(),
    m_drawCaretFlag(),
    m_pFocusedImage(nullptr),
    m_bUseControlCursor(false),
    m_bEnableWheelZoom(false),
    m_bEnableDefaultContextMenu(false),
    m_bDisableTextChangeEvent(false),
    m_maxNumber(INT_MAX),
    m_minNumber(INT_MIN),
    m_pSpinBox(nullptr),
    m_pClearButton(nullptr),
    m_pShowPasswordButton(nullptr),
    m_bReadOnly(false),
    m_bPasswordMode(false),
    m_bShowPassword(false),
    m_chPasswordChar(L'*'),
    m_bFlashPasswordChar(false),
    m_bNumberOnly(false),
    m_bWordWrap(false),
    m_bMultiLine(false),
    m_nLimitText(0),
    m_bModified(false),
    m_hAlignType(HorAlignType::kHorAlignLeft),
    m_vAlignType(VerAlignType::kVerAlignTop),
    m_nSelStartIndex(-1),
    m_nSelEndCharIndex(0),
    m_bHideSelection(false)
{
}

RichEdit::~RichEdit()
{
    if (m_pFocusedImage != nullptr) {
        delete m_pFocusedImage;
        m_pFocusedImage = nullptr;
    }
    m_pLimitChars.reset();
}

DString RichEdit::GetType() const { return DUI_CTR_RICHEDIT; }

void RichEdit::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("vscrollbar")) {
        //纵向滚动条
        if (strValue == _T("true")) {
            EnableScrollBar(true, GetHScrollBar() != NULL);
        }
        else {
            EnableScrollBar(false, GetHScrollBar() != NULL);
        }
    }
    else if (strName == _T("hscrollbar")) {
        //横向滚动条
        if (strValue == _T("true")) {
            EnableScrollBar(GetVScrollBar() != NULL, true);
        }
        else {
            EnableScrollBar(GetVScrollBar() != NULL, false);
        }
    }
    else if ((strName == _T("single_line")) || (strName == _T("singleline"))) {
        SetMultiLine(strValue != _T("true"));
    }
    else if ((strName == _T("multi_line")) || (strName == _T("multiline"))) {
        SetMultiLine(strValue == _T("true"));
    }
    else if (strName == _T("readonly")) {
        SetReadOnly(strValue == _T("true"));
    }
    else if (strName == _T("password")) {
        SetPassword(strValue == _T("true"));
    }
    else if (strName == _T("show_password")) {
        SetShowPassword(strValue == _T("true"));
    }
    else if (strName == _T("password_char")) {
        if (!strValue.empty()) {
            SetPasswordChar(strValue.front());
        }
    }
    else if (strName == _T("flash_password_char")) {
        SetFlashPasswordChar(strValue == _T("true"));
    }
    else if ((strName == _T("number_only")) || (strName == _T("number"))) {
        SetNumberOnly(strValue == _T("true"));
    }
    else if (strName == _T("max_number")) {
        SetMaxNumber(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("min_number")) {
        SetMinNumber(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("text_align")) {
        if (strValue.find(_T("left")) != DString::npos) {
            SetHAlignType(kHorAlignLeft);
        }
        if (strValue.find(_T("right")) != DString::npos) {
            SetHAlignType(kHorAlignRight);
        }
        if (strValue.find(_T("hcenter")) != DString::npos) {
            SetHAlignType(kHorAlignCenter);
        }

        if (strValue.find(_T("top")) != DString::npos) {
            SetVAlignType(kVerAlignTop);
        }
        if (strValue.find(_T("bottom")) != DString::npos) {
            SetVAlignType(kVerAlignBottom);
        }
        if (strValue.find(_T("vcenter")) != DString::npos) {
            SetVAlignType(kVerAlignCenter);
        }
    }
    else if ((strName == _T("text_padding")) || (strName == _T("textpadding"))) {
        UiPadding rcTextPadding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcTextPadding);
        SetTextPadding(rcTextPadding, true);
    }
    else if ((strName == _T("text_color")) || (strName == _T("normal_text_color")) || (strName == _T("normaltextcolor"))) {
        SetTextColor(strValue);
    }
    else if ((strName == _T("disabled_text_color")) || (strName == _T("disabledtextcolor"))) {
        SetDisabledTextColor(strValue);
    }
    else if ((strName == _T("caret_color")) || (strName == _T("caretcolor"))) {
        //设置光标的颜色
        SetCaretColor(strValue);
    }
    else if ((strName == _T("prompt_mode")) || (strName == _T("promptmode"))) {
        //提示模式
        m_bAllowPrompt = (strValue == _T("true")) ? true : false;
    }
    else if ((strName == _T("prompt_color")) || (strName == _T("promptcolor"))) {
        //提示文字的颜色
        m_sPromptColor = strValue;
    }
    else if ((strName == _T("prompt_text")) || (strName == _T("prompttext"))) {
        //提示文字
        SetPromptText(strValue);
    }
    else if ((strName == _T("prompt_text_id")) || (strName == _T("prompt_textid")) || (strName == _T("prompttextid"))) {
        //提示文字ID
        SetPromptTextId(strValue);
    }
    else if ((strName == _T("focused_image")) || (strName == _T("focusedimage"))) {
        SetFocusedImage(strValue);
    }
    else if (strName == _T("font")) {
        SetFontId(strValue);
    }
    else if (strName == _T("text")) {
        SetText(strValue);
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))) {
        SetTextId(strValue);
    }
    else if ((strName == _T("want_tab")) || (strName == _T("wanttab"))) {
        SetWantTab(strValue == _T("true"));
    }
    else if ((strName == _T("want_return")) || (strName == _T("want_return_msg")) || (strName == _T("wantreturnmsg"))) {
        SetWantReturn(strValue == _T("true"));
    }
    else if ((strName == _T("want_ctrl_return")) || (strName == _T("return_msg_want_ctrl")) || (strName == _T("returnmsgwantctrl"))) {
        SetWantCtrlReturn(strValue == _T("true"));
    }
    else if ((strName == _T("limit_text")) || (strName == _T("max_char")) || (strName == _T("maxchar"))) {
        //限制最多字符数
        SetLimitText(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("limit_chars")) {
        //限制允许输入哪些字符
        SetLimitChars(strValue);
    }
    else if (strName == _T("word_wrap")) {
        //是否自动换行
        SetWordWrap(strValue == _T("true"));
    }
    else if (strName == _T("no_caret_readonly")) {
        //只读模式，不显示光标
        SetNoCaretReadonly();
    }
    else if (strName == _T("default_context_menu")) {
        //是否使用默认的右键菜单
        SetEnableDefaultContextMenu(strValue == _T("true"));
    }
    else if (strName == _T("spin_class")) {
        SetSpinClass(strValue);
    }
    else if (strName == _T("clear_btn_class")) {
        SetClearBtnClass(strValue);
    }
    else if (strName == _T("show_passowrd_btn_class")) {
        SetShowPasswordBtnClass(strValue);
    }
    else if (strName == _T("wheel_zoom")) {
        //设置是否允许Ctrl + 滚轮来调整缩放比例
        SetEnableWheelZoom(strValue == _T("true"));
    }




    else if (strName == _T("zoom")) {
        //缩放比例：
        //设置缩放比例：设 wParam：缩放比例的分子，lParam：缩放比例的分母，
        // "wParam,lParam" 表示按缩放比例分子/分母显示的缩放，取值范围：1/64 < (wParam / lParam) < 64。
        // 举例：则："0,0"表示关闭缩放功能，"2,1"表示放大到200%，"1,2"表示缩小到50% 
        //UiSize zoomValue;
        //AttributeUtil::ParseSizeValue(strValue.c_str(), zoomValue);
        //if ((zoomValue.cx >= 0) && (zoomValue.cx <= 64) &&
        //    (zoomValue.cy >= 0) && (zoomValue.cy <= 64)) {
        //    m_richCtrl.SetZoom(zoomValue.cx, zoomValue.cy);
        //}
    }    
    else if ((strName == _T("auto_vscroll")) || (strName == _T("autovscroll"))) {
        //当用户在最后一行按 ENTER 时，自动将文本向上滚动一页。
        //if (m_pRichHost != nullptr) {
        //    m_pRichHost->SetAutoVScroll(strValue == _T("true"));
        //}
    }
    else if ((strName == _T("auto_hscroll")) || (strName == _T("autohscroll"))) {
        //当用户在行尾键入一个字符时，自动将文本向右滚动 10 个字符。
        //当用户按 Enter 时，控件会将所有文本滚动回零位置。
        //if (m_pRichHost != nullptr) {
        //    m_pRichHost->SetAutoHScroll(strValue == _T("true"));
        //}
    }
    else if ((strName == _T("rich_text")) || (strName == _T("rich"))) {
        //是否为富文本属性
        //SetRichText(strValue == _T("true"));
    }
    else if (strName == _T("auto_detect_url")) {
        //是否自动检测URL，如果是URL则显示为超链接
        //SetAutoURLDetect(strValue == _T("true"));
    }
    else if (strName == _T("allow_beep")) {
        //是否允许发出Beep声音
        //SetAllowBeep(strValue == _T("true"));
    }
    else if (strName == _T("save_selection")) {
        //如果 为 TRUE，则当控件处于非活动状态时，应保存所选内容的边界。
        //如果 为 FALSE，则当控件再次处于活动状态时，可以选择边界重置为 start = 0，length = 0。
        //SetSaveSelection(strValue == _T("true"));
    }
    else if (strName == _T("hide_selection")) {
        //是否隐藏选择内容
        //SetHideSelection(strValue == _T("true"));
    }
    else if (strName == _T("enable_drag_drop")) {
        //是否允许拖放操作
        //SetEnableDragDrop(strValue == _T("true"));
    }


    else {
        Box::SetAttribute(strName, strValue);
    }
}

void RichEdit::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();

    //设置字体和字体颜色
    DString fontId = GetFontId();
    if (fontId.empty()) {
        fontId = GlobalManager::Instance().Font().GetDefaultFontId();
        SetFontIdInternal(fontId);
    }

    ScrollBar* pHScrollBar = GetHScrollBar();
    if (pHScrollBar != nullptr) {
        pHScrollBar->SetScrollRange(0);
    }
    ScrollBar* pVScrollBar = GetVScrollBar();
    if (pVScrollBar != nullptr) {
        pVScrollBar->SetScrollRange(0);
    }
}

void RichEdit::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    UiPadding rcTextPadding = GetTextPadding();
    rcTextPadding = Dpi().GetScalePadding(rcTextPadding, nOldDpiScale);
    SetTextPadding(rcTextPadding, false);

    //更新字体大小
    //TODO:

    __super::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

bool RichEdit::IsWantTab() const
{
    return m_bWantTab;
}

void RichEdit::SetWantTab(bool bWantTab)
{
    m_bWantTab = bWantTab;
}

bool RichEdit::CanPlaceCaptionBar() const
{
    return true;
}

bool RichEdit::IsWantReturn() const
{
    return m_bWantReturn;
}

void RichEdit::SetWantReturn(bool bWantReturn)
{
    m_bWantReturn = bWantReturn;
}

bool RichEdit::IsWantCtrlReturn() const
{
    return m_bWantCtrlReturn;
}

void RichEdit::SetWantCtrlReturn(bool bWantCtrlReturn)
{
    m_bWantCtrlReturn = bWantCtrlReturn;
}

bool RichEdit::IsReadOnly() const
{
    return m_bReadOnly;
}

void RichEdit::SetReadOnly(bool bReadOnly)
{
    if (m_bReadOnly != bReadOnly) {
        m_bReadOnly = bReadOnly;
        Redraw();
    }    
}

bool RichEdit::IsPassword() const
{
    return m_bPasswordMode;
}

void RichEdit::SetPassword(bool bPassword)
{
    if (m_bPasswordMode != bPassword) {
        m_bPasswordMode = bPassword;
        Redraw();
    }
}

void RichEdit::SetShowPassword(bool bShow)
{
    if (m_bShowPassword != bShow) {
        m_bShowPassword = bShow;
        Redraw();
    }
}

bool RichEdit::IsShowPassword() const
{
    return m_bShowPassword;
}

void RichEdit::SetPasswordChar(wchar_t ch)
{
    if (m_chPasswordChar != ch) {
        m_chPasswordChar = ch;
        if (IsPassword()) {
            Redraw();
        }
    }
}

void RichEdit::SetFlashPasswordChar(bool bFlash)
{
    m_bFlashPasswordChar = bFlash;
}

bool RichEdit::IsFlashPasswordChar() const
{
    return m_bFlashPasswordChar;
}

bool RichEdit::IsNumberOnly() const
{
    return m_bNumberOnly;
}

void RichEdit::SetNumberOnly(bool bNumberOnly)
{
    //只影响输入，不影响其他方式（比如SetText等方式改变文本）
    m_bNumberOnly = bNumberOnly;
}

void RichEdit::SetMaxNumber(int32_t maxNumber)
{
    m_maxNumber = maxNumber;
}

int32_t RichEdit::GetMaxNumber() const
{
    return m_maxNumber;
}

void RichEdit::SetMinNumber(int32_t minNumber)
{
    m_minNumber = minNumber;
}

int32_t RichEdit::GetMinNumber() const
{
    return m_minNumber;
}

bool RichEdit::IsWordWrap() const
{
    return m_bWordWrap;
}

void RichEdit::SetWordWrap(bool bWordWrap)
{
    if (m_bWordWrap != bWordWrap) {
        m_bWordWrap = bWordWrap;
        Redraw();
    }
}

bool RichEdit::IsMultiLine() const
{
    return m_bMultiLine;
}

void RichEdit::SetMultiLine(bool bMultiLine)
{
    if (m_bMultiLine != bMultiLine) {
        m_bMultiLine = bMultiLine;
        Redraw();
    }
}

DString RichEdit::GetFontId() const
{
    return m_sFontId.c_str();
}

void RichEdit::SetFontId(const DString& strFontId)
{
    if (m_sFontId != strFontId) {
        m_sFontId = strFontId;
        SetFontIdInternal(strFontId);
        Redraw();
    }
}

void RichEdit::SetTextColor(const DString& dwTextColor)
{
    if (m_sTextColor != dwTextColor) {
        m_sTextColor = dwTextColor;
        if (IsEnabled()) {
            Redraw();
        }
    }    
}

DString RichEdit::GetTextColor() const
{
    if (!m_sTextColor.empty()) {
        return m_sTextColor.c_str();
    }
    else {
        return GlobalManager::Instance().Color().GetDefaultTextColor();
    }
}

void RichEdit::SetDisabledTextColor(const DString& dwTextColor)
{
    if (m_sDisabledTextColor != dwTextColor) {
        m_sDisabledTextColor = dwTextColor;
        if (!IsEnabled()) {
            Redraw();
        }
    }    
}

DString RichEdit::GetDisabledTextColor() const
{
    if (!m_sDisabledTextColor.empty()) {
        return m_sDisabledTextColor.c_str();
    }
    else {
        return GlobalManager::Instance().Color().GetDefaultDisabledTextColor();
    }
}

int32_t RichEdit::GetLimitText() const
{
    return m_nLimitText;
}

void RichEdit::SetLimitText(int32_t iChars)
{
    if (iChars < 0) {
        iChars = 0;
    }
    m_nLimitText = iChars;
}

DString RichEdit::GetLimitChars() const
{
    if (m_pLimitChars != nullptr) {
        return StringUtil::UTF16ToT(m_pLimitChars.get());
    }
    else {
        return DString();
    }
}

void RichEdit::SetLimitChars(const DString& limitChars)
{
    m_pLimitChars.reset();
    DStringW limitCharsW = StringUtil::TToUTF16(limitChars);
    if (!limitCharsW.empty()) {
        size_t nLen = limitCharsW.size() + 1;
        m_pLimitChars.reset(new wchar_t[nLen]);
        memset(m_pLimitChars.get(), 0, nLen * sizeof(wchar_t));
        StringUtil::StringCopy(m_pLimitChars.get(), nLen, limitCharsW.c_str());
    }
}

int32_t RichEdit::GetTextLength() const
{
    return (int32_t)m_text.size();
}

DString RichEdit::GetText() const
{
    return m_text + L"\n行1\r\n行2\r\n行3\n\n行4\nUTF16: TAB键:|\t|sdfkljAKLDFJKEWkldfjlk#$%&sdfs.dsj 中文字符串|\xD852\xDF62|\xD83D\xDC69|字符|\xD842\xDF20|\xD83D\xDE02|中文";
}

std::string RichEdit::GetUTF8Text() const
{
    std::string strOut = StringUtil::TToUTF8(GetText());
    return strOut;
}

void RichEdit::SetText(const DString& strText)
{
#ifdef DUILIB_UNICODE
    if (m_text != strText) {
        m_text = strText;
#ifdef _DEBUG
        std::vector<uint8_t> fileData;
        FileUtil::ReadFileData(FilePath(L"D:\\1.h"), fileData);
        fileData.push_back(0);
        fileData.push_back(0);
        m_text = StringUtil::UTF8ToUTF16((const char*)fileData.data());
#endif
        OnTextChanged();
    }    
#else
    DStringW text = StringUtil::UTF8ToUTF16(strText);
    if (m_text != text) {
        m_text.swap(text);
        OnTextChanged();
    }
#endif    
}

void RichEdit::SetTextNoEvent(const DString& strText)
{
    bool bOldValue = m_bDisableTextChangeEvent;
    m_bDisableTextChangeEvent = true;
    SetText(strText);
    m_bDisableTextChangeEvent = bOldValue;
}

void RichEdit::SetTextId(const DString& strTextId)
{
    DString strText = GlobalManager::Instance().Lang().GetStringViaID(strTextId);
    SetText(strText);
}

void RichEdit::SetUTF8Text( const std::string& strText )
{
    DString strOut = StringUtil::UTF8ToT(strText);
    SetText(strOut);
}

bool RichEdit::GetModify() const
{ 
    return m_bModified;
}

void RichEdit::SetModify(bool bModified)
{ 
    m_bModified = bModified;
}

void RichEdit::GetSel(int32_t& nStartChar, int32_t& nEndChar) const
{
    //TODO:
    LONG nStart = 0;
    LONG nEnd = 0;
    //m_richCtrl.GetSel(nStart, nEnd);
    nStartChar = nStart;
    nEndChar = nEnd;
}

int32_t RichEdit::SetSel(int32_t nStartChar, int32_t nEndChar)
{
    if ((nStartChar < 0) || (nStartChar >= (int32_t)m_text.size())){
        //无选择
        nStartChar = -1;
    }
    if (nEndChar >= (int32_t)m_text.size()) {
        //选择至文本结束
        nEndChar = -1;
    }
    bool bChanged = (m_nSelStartIndex != nStartChar) || (m_nSelEndCharIndex != nEndChar);
    m_nSelStartIndex = nStartChar;
    m_nSelEndCharIndex = nEndChar;
    int32_t nSelCount = 0;
    if ((nStartChar >= 0) && (nStartChar < (int32_t)m_text.size())){
        if (nEndChar < 0) {
            nSelCount = (int32_t)m_text.size() - nStartChar;
        }
        else if ((nEndChar > nStartChar) && (nEndChar < (int32_t)m_text.size())) {
            nSelCount = nEndChar - nStartChar;
        }
    }
    if (bChanged) {
        Redraw();
    }
    return nSelCount;
}

void RichEdit::ReplaceSel(const DString& lpszNewText, bool bCanUndo)
{
    //TODO
    //m_richCtrl.ReplaceSel(lpszNewText.c_str(), bCanUndo);
    
}

DString RichEdit::GetSelText() const
{
    DString text;
    //TODO
    //m_richCtrl.GetSelText(text);
    return text;
}

int32_t RichEdit::SetSelAll()
{
    return SetSel(0, -1);
}

void RichEdit::SetSelNone()
{
    SetSel(-1, 0);
}

DString RichEdit::GetTextRange(int32_t nStartChar, int32_t nEndChar) const
{
    //TODO
    return L"";
}

void RichEdit::HideSelection(bool bHideSelection, bool /*bChangeStyle*/)
{
    if (m_bHideSelection != bHideSelection) {
        m_bHideSelection = bHideSelection;
        Redraw();
    }
}

bool RichEdit::CanRedo() const
{
    //return m_richCtrl.CanRedo();
    return false;
}

bool RichEdit::Redo()
{
    //return m_richCtrl.Redo();
    return false;
}

bool RichEdit::CanUndo() const
{
    //return m_richCtrl.CanUndo();
    return false;
}

bool RichEdit::Undo()
{
    //return m_richCtrl.Undo();
    return false;
}

void RichEdit::Clear()
{
    //m_richCtrl.Clear();
}

void RichEdit::Copy()
{
    //m_richCtrl.Copy();
}

void RichEdit::Cut()
{
    //m_richCtrl.Cut();
}

void RichEdit::Paste()
{
    if (IsPasteLimited()) {
        return;
    }
    //m_richCtrl.Paste();
}

bool RichEdit::CanPaste() const
{
    if (IsPasteLimited()) {
        return false;
    }
    //return m_richCtrl.CanPaste(0);
    return false;
}

int32_t RichEdit::GetLineCount() const
{
    //return m_richCtrl.GetLineCount();
    return 0;
}

DString RichEdit::GetLine(int32_t nIndex, int32_t nMaxLength) const
{
    //TODO
    return L"";
}

int32_t RichEdit::LineIndex(int32_t nLine) const
{
    //return m_richCtrl.LineIndex(nLine);
    return 0;
}

int32_t RichEdit::LineLength(int32_t nLine) const
{
    //return m_richCtrl.LineLength(nLine);
    return 0;
}

bool RichEdit::LineScroll(int32_t nLines)
{
    //return m_richCtrl.LineScroll(nLines);
    return false;
}

int32_t RichEdit::LineFromChar(int32_t nIndex) const
{
    //return m_richCtrl.LineFromChar((LONG)nIndex);
    return 0;
}

UiPoint RichEdit::PosFromChar(int32_t lChar) const
{
    // POINT pt = m_richCtrl.PosFromChar(lChar);
    // return UiPoint(pt.x, pt.y);
    return UiPoint();
}

int32_t RichEdit::CharFromPos(UiPoint pt) const
{
    //POINT ptValue = { pt.x, pt.y };
    //return m_richCtrl.CharFromPos(ptValue);
    return 0;
}

void RichEdit::EmptyUndoBuffer()
{
    //m_richCtrl.EmptyUndoBuffer();
}

uint32_t RichEdit::SetUndoLimit(uint32_t nLimit)
{
    //return m_richCtrl.SetUndoLimit(nLimit);
    return 0;
}

HWND RichEdit::GetWindowHWND() const
{
    auto window = GetWindow();
    return window ? window->NativeWnd()->GetHWND() : nullptr;
}

HDC RichEdit::GetDrawDC() const
{
    auto window = GetWindow();
    return window ? window->NativeWnd()->GetPaintDC() : nullptr;
}

void RichEdit::SetImmStatus(BOOL bOpen)
{
    HWND hwnd = GetWindowHWND();
    if (hwnd == nullptr) {
        return;
    }
    // 失去焦点时关闭输入法
    HIMC hImc = ::ImmGetContext(hwnd);
    // 失去焦点是会把关联的输入法去掉，导致无法无法输入中文
    //::ImmAssociateContext(hwnd, bOpen ? hImc : NULL);
    if (hImc != NULL) {
        if (::ImmGetOpenStatus(hImc)) {
            if (!bOpen) {
                ::ImmSetOpenStatus(hImc, FALSE);
            }
        }
        else {
            if (bOpen) {
                ::ImmSetOpenStatus(hImc, TRUE);
            }
        }
        ::ImmReleaseContext(hwnd, hImc);
    }
}

// 多行非rich格式的richedit有一个滚动条bug，在最后一行是空行时，LineDown和SetScrollPos无法滚动到最后
// 引入iPos就是为了修正这个bug
void RichEdit::SetScrollPos(UiSize64 szPos)
{
    bool bRichText = false;
    int64_t cx = 0;
    int64_t cy = 0;
    ScrollBar* pVScrollBar = GetVScrollBar();
    ScrollBar* pHScrollBar = GetHScrollBar();
    if ((pVScrollBar != nullptr) && pVScrollBar->IsValid() ) {
        int64_t iLastScrollPos = pVScrollBar->GetScrollPos();
        pVScrollBar->SetScrollPos(szPos.cy);
        cy = pVScrollBar->GetScrollPos() - iLastScrollPos;
    }
    if ((pHScrollBar != nullptr) && pHScrollBar->IsValid() ) {
        int64_t iLastScrollPos = pHScrollBar->GetScrollPos();
        pHScrollBar->SetScrollPos(szPos.cx);
        cx = pHScrollBar->GetScrollPos() - iLastScrollPos;
    }
    if( cy != 0 ) {
        int64_t iPos = 0;
        if (!bRichText && (pVScrollBar != nullptr) && pVScrollBar->IsValid()) {
            iPos = pVScrollBar->GetScrollPos();
        }
        WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, (pVScrollBar != nullptr) ? pVScrollBar->GetScrollPos() : 0);
       // m_richCtrl.TxSendMessage(WM_VSCROLL, wParam, 0L);
        if(!bRichText && (pVScrollBar != nullptr) && pVScrollBar->IsValid() ) {
            if (cy > 0 && pVScrollBar->GetScrollPos() <= iPos) {
                pVScrollBar->SetScrollPos(iPos);
            }
        }
    }
    if( cx != 0 ) {
        WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, (pHScrollBar != nullptr) ? pHScrollBar->GetScrollPos() : 0);
       // m_richCtrl.TxSendMessage(WM_HSCROLL, wParam, 0L);
    }
}

void RichEdit::LineUp()
{
    __super::LineUp();
}

void RichEdit::LineDown()
{
    __super::LineUp();
}

void RichEdit::PageUp()
{
    __super::PageUp();
}

void RichEdit::PageDown()
{
    __super::PageDown();
}

void RichEdit::HomeUp()
{
    __super::HomeUp();
}

void RichEdit::EndDown()
{
    __super::EndDown();
}

void RichEdit::LineLeft()
{
    __super::LineLeft();
}

void RichEdit::LineRight()
{
    __super::LineRight();
}

void RichEdit::PageLeft()
{
    __super::PageLeft();
}

void RichEdit::PageRight()
{
    __super::PageRight();
}

void RichEdit::HomeLeft()
{
    __super::HomeLeft();
}

void RichEdit::EndRight()
{
    __super::EndRight();
}

void RichEdit::SetEnabled(bool bEnable)
{
    bool bChanged = IsEnabled() != bEnable;
    __super::SetEnabled(bEnable);
    if (IsEnabled()) {
        SetState(kControlStateNormal);
    }
    else {
        SetState(kControlStateDisabled);
    }
    if (bChanged) {
        Redraw();
    }
}

uint32_t RichEdit::GetControlFlags() const
{
    return IsEnabled() && IsAllowTabStop() ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

void RichEdit::HandleEvent(const EventArgs& msg)
{
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        else {
            __super::HandleEvent(msg);
        }
        return;
    }
    if (msg.eventType == kEventMouseMove) {
        OnMouseMessage(WM_MOUSEMOVE, msg);
        return;
    }
    if (msg.eventType == kEventMouseWheel) {
        if (IsKeyDown(msg, ModifierKey::kControl) && IsEnableWheelZoom()) {
            //Ctrl + 滚轮：缩放功能
            OnMouseMessage(WM_MOUSEWHEEL, msg);
            int32_t nNum = 0;
            int32_t nDen = 0;
           // m_richCtrl.GetZoom(nNum, nDen);
            SendEvent(kEventZoom, (WPARAM)nNum, (LPARAM)nDen);
        }
        else {
            ScrollBox::HandleEvent(msg);
        }
        return;
    }

    if (msg.eventType == kEventMouseButtonDown) {
        AdjustCaretPos(msg.ptMouse);        
        OnMouseMessage(WM_LBUTTONDOWN, msg);
        return;
    }
    if (msg.eventType == kEventMouseButtonUp) {
        if (IsEnabled() && !m_bSelAllEver) {
            m_bSelAllEver = true;
            if (m_bSelAllOnFocus) {
                SetSelAll();
                if (IsMultiLine()) {
                    HomeUp();
                }
                else {
                    HomeLeft();
                }
            }
        }

        OnMouseMessage(WM_LBUTTONUP, msg);
        return;
    }
    if (msg.eventType == kEventMouseDoubleClick) {
        if (IsReadOnly()) {
            SetSelAll();
            return;
        }

        OnMouseMessage(WM_LBUTTONDBLCLK, msg);
        return;
    }
    if (msg.eventType == kEventMouseRButtonDown) {
        AdjustCaretPos(msg.ptMouse);

        OnMouseMessage(WM_RBUTTONDOWN, msg);
        return;
    }
    if (msg.eventType == kEventMouseRButtonUp) {
        OnMouseMessage(WM_RBUTTONUP, msg);
        return;
    }
    else if (msg.eventType == kEventKeyDown) {
        OnKeyDown(msg);
        return;
    }
    ScrollBox::HandleEvent(msg);
}

bool RichEdit::OnSetCursor(const EventArgs& msg)
{
    if (m_bUseControlCursor) {
        //使用Control设置的光标
        return __super::OnSetCursor(msg);
    }

    /*if (m_pRichHost && m_pRichHost->SetCursor(nullptr, &msg.ptMouse)) {
        return true;
    }
    else*/ {
        SetCursor(IsReadOnly() ? CursorType::kCursorArrow : CursorType::kCursorIBeam);
        return true;
    }
}

bool RichEdit::OnSetFocus(const EventArgs& /*msg*/)
{
    //if (m_pRichHost != nullptr) {
    //    m_pRichHost->OnTxInPlaceActivate(NULL);
    //    m_richCtrl.TxSendMessage(WM_SETFOCUS, 0, 0);
    //    ShowCaret(true);
    //}
    AdjustCaretPos(UiPoint(-1,-1));
    ShowCaret(true);
    SetImmStatus(TRUE);

#ifdef DUILIB_BUILD_FOR_SDL
    if (IsVisible() && !IsReadOnly() && IsEnabled()) {
        SDL_StartTextInput((SDL_Window*)GetWindow()->NativeWnd()->GetWindowHandle());
    }
#endif

    if ((m_pClearButton != nullptr) && !IsReadOnly()){
        m_pClearButton->SetFadeVisible(true);
    }
    if ((m_pShowPasswordButton != nullptr) && IsPassword() && !IsShowPassword()) {
        m_pShowPasswordButton->SetFadeVisible(true);
    }
    Invalidate();
    return true;
}

bool RichEdit::OnKillFocus(const EventArgs& /*msg*/)
{
    //if (m_pRichHost) {
    //    m_pRichHost->OnTxInPlaceActivate(NULL);
    //    m_richCtrl.TxSendMessage(WM_KILLFOCUS, 0, 0);
    //    ShowCaret(false);
    //}
    ShowCaret(false);
    m_bSelAllEver = false;
    if (m_bNoSelOnKillFocus && IsReadOnly() && IsEnabled()) {
        SetSelNone();
    }
    if (m_bSelAllOnFocus && IsEnabled()) {
        SetSelNone();
    }

    SetImmStatus(FALSE);

#ifdef DUILIB_BUILD_FOR_SDL
    if (IsVisible() && !IsReadOnly() && IsEnabled()) {
        SDL_StopTextInput((SDL_Window*)GetWindow()->NativeWnd()->GetWindowHandle());
    }
#endif

    if (m_pClearButton != nullptr) {
        m_pClearButton->SetFadeVisible(false);
    }
    if (m_pShowPasswordButton != nullptr) {
        m_pShowPasswordButton->SetFadeVisible(false);
    }
    Invalidate();
    return true;
}

bool RichEdit::OnChar(const EventArgs& msg)
{
    //TAB
    if (::GetKeyState(VK_TAB) < 0 && !m_bWantTab) {
        SendEvent(kEventTab);
        return true;
    }
    //Number
    if (IsNumberOnly()) {
        if (msg.vkCode < '0' || msg.vkCode > '9') {
            if (msg.vkCode == _T('-')) {
                if (GetTextLength() > 0) {
                    //不是第一个字符，禁止输入负号
                    return true;
                }
                else if (GetMinNumber() >= 0) {
                    //最小数字是0或者正数，禁止输入符号
                    return true;
                }
            }
            else {
                return true;
            }
        }
    }

    //限制允许输入的字符
    if (m_pLimitChars != nullptr) {
        if (!IsInLimitChars((wchar_t)msg.vkCode)) {
            //字符不在列表里面，禁止输入
            return true;
        }
    }
#ifdef DUILIB_BUILD_FOR_SDL
    WPARAM wParam = msg.vkCode;
    LPARAM lParam = 0;
#endif
    //m_richCtrl.TxSendMessage(WM_CHAR, wParam, lParam); 
    return true;
}

bool RichEdit::IsInLimitChars(wchar_t charValue) const
{
    //返回false时：禁止输入
    if (m_pLimitChars == nullptr) {
        return true;
    }
    const wchar_t* ch = m_pLimitChars.get();
    if ((ch == nullptr) || (*ch == L'\0')) {
        return true;
    }
    bool bInLimitChars = false;
    while (*ch != L'\0') {
        if (*ch == charValue) {
            bInLimitChars = true;
            break;
        }
        ++ch;
    }
    return bInLimitChars;
}

bool RichEdit::IsPasteLimited() const
{
    if (m_pLimitChars != nullptr) {
        //有设置限制字符
        DStringW strClipText;
        GetClipboardText(strClipText);
        if (!strClipText.empty()) {
            size_t count = strClipText.size();
            for (size_t index = 0; index < count; ++index) {
                if (strClipText[index] == L'\0') {
                    break;
                }
                if (!IsInLimitChars(strClipText[index])) {
                    //有字符不在列表里面，禁止粘贴
                    return true;
                }
            }
        }
    }
    else if (IsNumberOnly()) {
        //数字模式
        DStringW strClipText;
        GetClipboardText(strClipText);
        if (!strClipText.empty()) {
            size_t count = strClipText.size();
            for (size_t index = 0; index < count; ++index) {
                if (strClipText[index] == L'\0') {
                    break;
                }
                if (strClipText[index] == L'-') {
                    if ((index == 0) && (strClipText.size() > 1)) {
                        //允许第一个字符是负号
                        continue;
                    }
                }
                if ((strClipText[index] > L'9') || (strClipText[index] < L'0')) {
                    //有不是数字的字符，禁止粘贴
                    return true;
                }
            }
        }
    }
    return false;
}

bool RichEdit::OnKeyDown(const EventArgs& msg)
{
    if (msg.vkCode == kVK_RETURN && ::GetKeyState(VK_SHIFT) >= 0)    {
        if (m_bWantReturn && ((m_bWantCtrlReturn && ::GetKeyState(VK_CONTROL) < 0) ||
            (!m_bWantCtrlReturn && ::GetKeyState(VK_CONTROL) >= 0))) {
            SendEvent(kEventReturn);
            return true;
        }
    }
    else if ((msg.vkCode == 'V') && (::GetKeyState(VK_CONTROL) < 0)) {
        if (IsPasteLimited()) {
            return true;
        }
    }

    WPARAM wParam = msg.wParam;
    LPARAM lParam = msg.lParam;
#ifdef DUILIB_BUILD_FOR_SDL
    wParam = msg.vkCode;
    lParam = 0;
#endif

    //m_richCtrl.TxSendMessage(WM_KEYDOWN, wParam, lParam);
    return true;
}

bool RichEdit::OnImeStartComposition(const EventArgs& /*msg*/)
{
    HWND hWnd = GetWindowHWND();
    if (hWnd == nullptr) {
        return true;
    }

    HIMC hImc = ::ImmGetContext(hWnd);
    if (hImc == nullptr) {
        return true;
    }

    COMPOSITIONFORM    cfs = { 0, };
    UiPoint ptScrollOffset = GetScrollOffsetInScrollBox();
    POINT pt;
    pt.x = m_iCaretPosX - ptScrollOffset.x;
    pt.y = m_iCaretPosY - ptScrollOffset.y;

    //pt.y += (m_iCaretHeight + lf.lfHeight) / 4;
    cfs.dwStyle = CFS_POINT;
    if (pt.x < 1) {
        pt.x = 1;
    }
    if (pt.y < 1) {
        pt.y = 1;
    }
    cfs.ptCurrentPos = pt;
    ::ImmSetCompositionWindow(hImc, &cfs);
    ::ImmReleaseContext(hWnd, hImc);
    m_bIsComposition = true;
    return true;
}

bool RichEdit::OnImeEndComposition(const EventArgs& /*msg*/)
{
    m_bIsComposition = false;
    return true;
}

void RichEdit::OnMouseMessage(uint32_t uMsg, const EventArgs& msg)
{
    UiPoint pt = msg.ptMouse;
    pt.Offset(GetScrollOffsetInScrollBox());
    WPARAM wParam = msg.wParam;
#ifdef DUILIB_BUILD_FOR_SDL
    wParam = 0;
    if (IsKeyDown(msg, ModifierKey::kControl)) {
        wParam |= MK_CONTROL;
    }
    if (IsKeyDown(msg, ModifierKey::kShift)) {
        wParam |= MK_SHIFT;
    }
#endif
    //m_richCtrl.TxSendMessage(uMsg, wParam, MAKELPARAM(pt.x, pt.y));
}

void RichEdit::Paint(IRender* pRender, const UiRect& rcPaint)
{
    if (pRender == nullptr) {
        return;
    }
    //必须不使用缓存，否则绘制异常
    ASSERT(IsUseCache() == false);

    bool bNeedPaint = true;
    if (pRender->IsClipEmpty()) {
        bNeedPaint = false;
    }    
    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
        bNeedPaint = false;
    }

    if (bNeedPaint) {
        Control::Paint(pRender, rcPaint);
    }

    if (bNeedPaint) {
        //计算区域
        CalcTextRects();

        //绘制
        std::vector<RichTextData> richTextDataList;
        if (GetRichTextForDraw(richTextDataList)) {
            UiRect rc = GetRect();
            rc.Deflate(GetControlPadding());
            rc.Deflate(GetTextPadding());

            UiSize scrollPos = GetScrollOffset();
            rc.Offset(-scrollPos.cx, -scrollPos.cy);

            IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
            ASSERT(pRenderFactory != nullptr);
            pRender->DrawRichText(rc, pRenderFactory, richTextDataList, (uint8_t)GetAlpha());
        }
    }
}

void RichEdit::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
        return;
    }

    PaintCaret(pRender, rcPaint);

    ScrollBar* pVScrollBar = GetVScrollBar();
    ScrollBar* pHScrollBar = GetHScrollBar();
    if (m_items.size() > 0) {
        UiRect rc = GetRectWithoutPadding();
        if ((pVScrollBar != nullptr) && pVScrollBar->IsValid()) {
            rc.right -= pVScrollBar->GetFixedWidth().GetInt32();
        }
        if ((pHScrollBar != nullptr) && pHScrollBar->IsValid()) {
            rc.bottom -= pHScrollBar->GetFixedHeight().GetInt32();
        }

        if (!UiRect::Intersect(rcTemp, rcPaint, rc)) {
            for (auto it = m_items.begin(); it != m_items.end(); ++it) {
                auto pControl = *it;
                if ((pControl == nullptr) || !pControl->IsVisible()) {
                    continue;
                }
                UiRect controlPos = pControl->GetPos();
                if (!UiRect::Intersect(rcTemp, rcPaint, controlPos)) {
                    continue;
                }
                if (pControl->IsFloat()) {
                    if (!UiRect::Intersect(rcTemp, GetRect(), controlPos)) {
                        continue;
                    }
                    pControl->AlphaPaint(pRender, rcPaint);
                }
            }
        }
        else {
            AutoClip childClip(pRender, rcTemp);
            for (auto it = m_items.begin(); it != m_items.end(); ++it) {
                auto pControl = *it;
                if ((pControl == nullptr) || !pControl->IsVisible()) {
                    continue;
                }
                UiRect controlPos = pControl->GetPos();
                if (!UiRect::Intersect(rcTemp, rcPaint, controlPos)) {
                    continue;
                }
                if (pControl->IsFloat()) {
                    if (!UiRect::Intersect(rcTemp, GetRect(), controlPos)) {
                        continue;
                    }
                    pControl->AlphaPaint(pRender, rcPaint);
                }
                else {
                    if (!UiRect::Intersect(rcTemp, rc, controlPos)) {
                        continue;
                    }
                    pControl->AlphaPaint(pRender, rcPaint);
                }
            }
        }
    }

    if ((pVScrollBar != nullptr) && pVScrollBar->IsVisible()) {
        UiRect verBarPos = pVScrollBar->GetPos();
        if (UiRect::Intersect(rcTemp, rcPaint, verBarPos)) {
            pVScrollBar->AlphaPaint(pRender, rcPaint);
        }
    }

    if ((pHScrollBar != nullptr) && pHScrollBar->IsVisible()) {
        UiRect horBarPos = pHScrollBar->GetPos();
        if (UiRect::Intersect(rcTemp, rcPaint, horBarPos)) {
            pHScrollBar->AlphaPaint(pRender, rcPaint);
        }
    }
}

bool RichEdit::CreateCaret(int32_t xWidth, int32_t yHeight)
{
    m_iCaretWidth = xWidth;
    m_iCaretHeight = yHeight;
    return true;
}

bool RichEdit::ShowCaret(bool fShow)
{
    if (fShow) {
        m_bIsCaretVisiable = true;
        m_drawCaretFlag.Cancel();
        std::function<void()> closure = UiBind(&RichEdit::ChangeCaretVisiable, this);
        GlobalManager::Instance().Timer().AddTimer(m_drawCaretFlag.GetWeakFlag(), closure, 500);

        UiRect rc = GetRect();
        SDL_Rect sdlRect;
        sdlRect.x = m_iCaretPosX;
        sdlRect.y = m_iCaretPosY;
        sdlRect.w = rc.right - sdlRect.x;
        sdlRect.h = m_nLineHeight; //高度设置与行高相同
        ASSERT(m_nLineHeight > 0);
        int32_t nCursorOffset = m_iCaretWidth + Dpi().GetScaleInt(1); //输入法的候选框与光标当前位置的距离（水平方向）, 避免遮盖光标
        SDL_SetTextInputArea((SDL_Window*)GetWindow()->NativeWnd()->GetWindowHandle(), &sdlRect, nCursorOffset);
    }
    else {
        m_bIsCaretVisiable = false;
        m_drawCaretFlag.Cancel();
    }

    Invalidate();
    return true;
}

void RichEdit::SetCaretColor(const DString& dwColor)
{
    m_sCaretColor = dwColor;
}

DString RichEdit::GetCaretColor()
{
    return m_sCaretColor.c_str();
}

UiRect RichEdit::GetCaretRect()
{
    UiRect rc = { m_iCaretPosX, m_iCaretPosY, m_iCaretPosX + m_iCaretWidth, m_iCaretPosY + m_iCaretHeight };
    return rc;
}

bool RichEdit::SetCaretPos(int32_t x, int32_t y)
{
    bool bShow = true;//!m_richCtrl.HasSelText()
    m_iCaretPosX = x;
    m_iCaretPosY = y;
    ShowCaret(bShow);
    return true;
}

void RichEdit::ChangeCaretVisiable()
{
    m_bIsCaretVisiable = !m_bIsCaretVisiable;
    Invalidate();
}

void RichEdit::PaintCaret(IRender* pRender, const UiRect& /*rcPaint*/)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (IsReadOnly() && m_bNoCaretReadonly) {
        return;
    }

    if (m_bIsCaretVisiable && !m_bIsComposition) {
        UiRect rect(m_iCaretPosX, m_iCaretPosY, m_iCaretPosX, m_iCaretPosY + m_iCaretHeight);
        UiColor dwClrColor(0xff000000);
        if (!m_sCaretColor.empty()) {
            dwClrColor = this->GetUiColor(m_sCaretColor.c_str());
        }
        pRender->DrawLine(UiPoint(rect.left, rect.top), UiPoint(rect.right, rect.bottom), dwClrColor, m_iCaretWidth);
    }
}

void RichEdit::SetPromptMode(bool bPrompt)
{
    if(bPrompt == m_bAllowPrompt)
        return;
    m_bAllowPrompt = bPrompt;
    Invalidate();
}

DString RichEdit::GetPromptText() const
{
    DString strText = m_sPromptText.c_str();
    if (strText.empty() && !m_sPromptTextId.empty()) {
        strText = GlobalManager::Instance().Lang().GetStringViaID(m_sPromptTextId.c_str());
    }

    return strText;
}

std::string RichEdit::GetUTF8PromptText() const
{
    std::string strOut = StringUtil::TToUTF8(GetPromptText());
    return strOut;
}

void RichEdit::SetPromptText(const DString& strText)
{
    if (m_sPromptText != strText) {
        m_sPromptText = strText;
        Invalidate();
    }
}

void RichEdit::SetUTF8PromptText(const std::string& strText)
{
    DString strOut = StringUtil::UTF8ToT(strText);
    SetPromptText(strOut);
}

void RichEdit::SetPromptTextId(const DString& strTextId)
{
    if (m_sPromptTextId == strTextId) {
        m_sPromptTextId = strTextId;
        Invalidate();
    }
}

void RichEdit::SetUTF8PromptTextId(const std::string& strTextId)
{
    DString strOut = StringUtil::UTF8ToT(strTextId);
    SetPromptTextId(strOut);
}

void RichEdit::PaintPromptText(IRender* pRender)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (GetTextLength() != 0) {
        return;
    }

    /*if (m_pRichHost == nullptr) {
        return;
    }

    DString strPrompt = GetPromptText();
    if (strPrompt.empty() || m_sPromptColor.empty()) {
        return;
    }

    UiRect rc;
    m_pRichHost->GetControlRect(&rc);
    UiColor dwClrColor = GetUiColor(m_sPromptColor.c_str());
    UINT dwStyle = TEXT_NOCLIP;
    pRender->DrawString(rc, strPrompt, dwClrColor, GetIFontById(m_sFontId.c_str()), dwStyle);*/
}

DString RichEdit::GetFocusedImage()
{
    if (m_pFocusedImage != nullptr) {
        return m_pFocusedImage->GetImageString();
    }
    return DString();
}

void RichEdit::SetFocusedImage( const DString& strImage )
{
    if (m_pFocusedImage == nullptr) {
        m_pFocusedImage = new Image;
    }
    m_pFocusedImage->SetImageString(strImage, Dpi());
    Invalidate();
}

void RichEdit::PaintStateImages(IRender* pRender)
{
    if (IsReadOnly()) {
        return;
    }

    if (IsFocused()) {
        if (m_pFocusedImage != nullptr) {
            PaintImage(pRender, m_pFocusedImage);
        }        
        PaintPromptText(pRender);
    }
    else {
        __super::PaintStateImages(pRender);
        PaintPromptText(pRender);
    }
}

void RichEdit::SetNoSelOnKillFocus(bool bNoSel)
{
    m_bNoSelOnKillFocus = bNoSel;
}

void RichEdit::SetSelAllOnFocus(bool bSelAll)
{
    m_bSelAllOnFocus = bSelAll;
}

void RichEdit::SetNoCaretReadonly()
{
    m_bNoCaretReadonly = true;
}

void RichEdit::ClearImageCache()
{
    __super::ClearImageCache();
    if (m_pFocusedImage != nullptr) {
        m_pFocusedImage->ClearImageCache();
    }    
}

void RichEdit::SetTextPadding(UiPadding padding, bool bNeedDpiScale)
{
    ASSERT((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0));
    if ((padding.left < 0) || (padding.top < 0) ||
        (padding.right < 0) || (padding.bottom < 0)) {
        return;
    }
    if (bNeedDpiScale) {
        Dpi().ScalePadding(padding);
    }
    if (!GetTextPadding().Equals(padding)) {
        m_rcTextPadding.left = TruncateToUInt16(padding.left);
        m_rcTextPadding.top = TruncateToUInt16(padding.top);
        m_rcTextPadding.right = TruncateToUInt16(padding.right);
        m_rcTextPadding.bottom = TruncateToUInt16(padding.bottom);
        RelayoutOrRedraw();
    }
}

UiPadding RichEdit::GetTextPadding() const
{
    return UiPadding(m_rcTextPadding.left, m_rcTextPadding.top, m_rcTextPadding.right, m_rcTextPadding.bottom);
}

void RichEdit::SetUseControlCursor(bool bUseControlCursor)
{
    m_bUseControlCursor = bUseControlCursor;
}

void RichEdit::GetClipboardText(DStringW& out )
{
    out.clear();
    BOOL ret = ::OpenClipboard(NULL);
    if(ret) {
        if(::IsClipboardFormatAvailable(CF_UNICODETEXT)) {
            HANDLE h = ::GetClipboardData(CF_UNICODETEXT);
            if(h != INVALID_HANDLE_VALUE) {
                wchar_t* buf = (wchar_t*)::GlobalLock(h);
                if(buf != NULL)    {
                    DStringW str(buf, GlobalSize(h)/sizeof(wchar_t));
                    out = str;
                    ::GlobalUnlock(h);
                }
            }
        }
        else if(::IsClipboardFormatAvailable(CF_TEXT)) {
            HANDLE h = ::GetClipboardData(CF_TEXT);
            if(h != INVALID_HANDLE_VALUE) {
                char* buf = (char*)::GlobalLock(h);
                if(buf != NULL)    {
                    std::string str(buf, GlobalSize(h));
                    out = StringUtil::MBCSToUnicode(str);
                    ::GlobalUnlock(h);
                }
            }
        }
        ::CloseClipboard();
    }
}

void RichEdit::AttachSelChange(const EventCallback& callback)
{ 
    AttachEvent(kEventSelChange, callback); 
    /*uint32_t oldEventMask = m_richCtrl.GetEventMask();
    if (!(oldEventMask & ENM_SELCHANGE)) {
        m_richCtrl.SetEventMask(oldEventMask | ENM_SELCHANGE);
        ASSERT(m_richCtrl.GetEventMask() & ENM_SELCHANGE);
        ASSERT(m_richCtrl.GetEventMask() & ENM_CHANGE);
        ASSERT(m_richCtrl.GetEventMask() & ENM_LINK);
    }  */  
}

void RichEdit::SetEnableWheelZoom(bool bEnable)
{
    m_bEnableWheelZoom = bEnable;
}

bool RichEdit::IsEnableWheelZoom(void) const
{
    return m_bEnableWheelZoom;
}

void RichEdit::SetEnableDefaultContextMenu(bool bEnable)
{
    if (m_bEnableDefaultContextMenu != bEnable) {
        m_bEnableDefaultContextMenu = bEnable;
        if (bEnable) {
            AttachContextMenu([this](const ui::EventArgs& args) {
                if (args.eventType == ui::kEventContextMenu) {
                    ui::UiPoint pt = args.ptMouse;
                    if ((pt.x != -1) && (pt.y != -1)) {
                        //鼠标右键点击产生的上下文菜单                        
                        ShowPopupMenu(pt);
                    }
                    else {
                        //按Shift + F10，由系统产生上下文菜单
                        pt = { 100, 100 };
                        ShowPopupMenu(pt);
                    }
                }
                return true;
                });
        }
        else {
            DetachEvent(kEventContextMenu);
        }
    }
}

bool RichEdit::IsEnableDefaultContextMenu() const
{
    return m_bEnableDefaultContextMenu;
}

void RichEdit::ShowPopupMenu(const ui::UiPoint& point)
{
    RichEdit* pRichEdit = this;
    if ((pRichEdit == nullptr) || !pRichEdit->IsEnabled() || pRichEdit->IsPassword()) {
        return;
    }

    //如果没有选中文本，则将光标切换到当前点击的位置
    int32_t nStartChar = 0; 
    int32_t nEndChar = 0;
    pRichEdit->GetSel(nStartChar, nEndChar);
    if (nStartChar == nEndChar) {
        //TODO:
        int32_t pos = 0;// pRichEdit->m_richCtrl.CharFromPos(POINT(point.x, point.y));
        if (pos > 0) {
            pRichEdit->SetSel(pos, pos);
            pRichEdit->GetSel(nStartChar, nEndChar);
        }
    }
    
    DString skinFolder = _T("public/menu/");
    Menu* menu = new Menu(GetWindow());//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    menu->SetSkinFolder(skinFolder);
    DString xml(_T("rich_edit_menu.xml"));

    //菜单弹出位置的坐标应为屏幕坐标
    UiPoint pt = point;
    ClientToScreen(pt);
    menu->ShowMenu(xml, pt);

    ui::MenuItem* menu_item = nullptr;
    //更新命令状态，并添加菜单命令响应
    bool hasSelText = nEndChar > nStartChar ? true : false;
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_copy")));
    if (menu_item != nullptr) {
        if (!hasSelText) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Copy();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_cut")));
    if (menu_item != nullptr) {
        if (!hasSelText) {
            menu_item->SetEnabled(false);
        }
        else if (pRichEdit->IsReadOnly()) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Cut();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_paste")));
    if (menu_item != nullptr) {
        if (!pRichEdit->CanPaste()) {
            menu_item->SetEnabled(false);
        }
        else if (pRichEdit->IsReadOnly()) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Paste();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_del")));
    if (menu_item != nullptr) {
        if (!hasSelText) {
            menu_item->SetEnabled(false);
        }
        else if (pRichEdit->IsReadOnly()) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Clear();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_sel_all")));
    if (menu_item != nullptr) {
        if ((nStartChar == 0) && (nEndChar == pRichEdit->GetTextLength())) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->SetSelAll();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_undo")));
    if (menu_item != nullptr) {
        if (!pRichEdit->CanUndo()) {
            menu_item->SetEnabled(false);
        }
        else if (pRichEdit->IsReadOnly()) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Undo();
            return true;
            });
    }
    menu_item = dynamic_cast<ui::MenuItem*>(menu->FindControl(_T("edit_menu_redo")));
    if (menu_item != nullptr) {
        if (!pRichEdit->CanRedo()) {
            menu_item->SetEnabled(false);
        }
        else if (pRichEdit->IsReadOnly()) {
            menu_item->SetEnabled(false);
        }
        menu_item->AttachClick([pRichEdit](const ui::EventArgs& /*args*/) {
            pRichEdit->Redo();
            return true;
            });
    }
}

void RichEdit::OnTextChanged()
{
    if (IsNumberOnly() && ((GetMinNumber() != INT_MIN) || (GetMaxNumber() != INT_MAX))) {
        //数字模式，检查文本对应的数字是否在范围内
        DString text = GetText();
        if (!text.empty()) {
            int64_t n = StringUtil::StringToInt64(text);
            if (n < GetMinNumber()) {
                //超过最小数字，进行修正
                int32_t newValue = GetMinNumber();
                SetTextNoEvent(StringUtil::Printf(_T("%d"), newValue));
                if (!m_bDisableTextChangeEvent) {
                    SendEvent(kEventTextChange);
                }
                return;
            }
            else if (n > GetMaxNumber()) {
                //超过最大数字，进行修正
                int32_t newValue = GetMaxNumber();
                SetTextNoEvent(StringUtil::Printf(_T("%d"), newValue));
                if (!m_bDisableTextChangeEvent) {
                    SendEvent(kEventTextChange);
                }
                return;
            }
        }
    }
    if (!m_bDisableTextChangeEvent) {
        SendEvent(kEventTextChange);
    }
    SetModify(true);
}

bool RichEdit::SetSpinClass(const DString& spinClass)
{
    DString spinBoxClass;
    DString spinBtnUpClass;
    DString spinBtnDownClass;
    std::list<DString> classNames = StringUtil::Split(spinClass, _T(","));
    if (classNames.size() == 3) {
        auto iter = classNames.begin();
        spinBoxClass = *iter++;
        spinBtnUpClass = *iter++;
        spinBtnDownClass = *iter++;
    }
    if (!spinClass.empty()) {
        ASSERT(!spinBoxClass.empty() && !spinBtnUpClass.empty() && !spinBtnDownClass.empty());
    }
    if (!spinBoxClass.empty() && !spinBtnUpClass.empty() && !spinBtnDownClass.empty()) {        
        Button* pUpButton = nullptr;
        Button* pDownButton = nullptr;
        if (m_pSpinBox == nullptr) {
            m_pSpinBox = new VBox(GetWindow());
            AddItem(m_pSpinBox);

            pUpButton = new Button(GetWindow());
            m_pSpinBox->AddItem(pUpButton);

            pDownButton = new Button(GetWindow());
            m_pSpinBox->AddItem(pDownButton);
        }
        else {
            pUpButton = dynamic_cast<Button*>(m_pSpinBox->GetItemAt(0));
            pDownButton = dynamic_cast<Button*>(m_pSpinBox->GetItemAt(1));            
        }

        ASSERT((pUpButton != nullptr) && (pDownButton != nullptr));
        if ((pUpButton == nullptr) || (pDownButton == nullptr)) {
            RemoveItem(m_pSpinBox);
            m_pSpinBox = nullptr;
            return false;
        }
        m_pSpinBox->SetClass(spinBoxClass);
        pUpButton->SetClass(spinBtnUpClass);
        pDownButton->SetClass(spinBtnDownClass);
        
        //挂载事件处理
        pUpButton->DetachEvent(kEventClick);
        pUpButton->AttachClick([this](const EventArgs& /*args*/){
            AdjustTextNumber(1);
            return true;
            });

        pUpButton->DetachEvent(kEventMouseButtonDown);
        pUpButton->AttachButtonDown([this](const EventArgs& /*args*/) {
            StartAutoAdjustTextNumberTimer(1);
            return true;
            });

        pUpButton->DetachEvent(kEventMouseButtonUp);
        pUpButton->AttachButtonUp([this](const EventArgs& /*args*/) {
            StopAutoAdjustTextNumber();
            return true;
            });

        pUpButton->DetachEvent(kEventMouseLeave);
        pUpButton->AttachMouseLeave([this](const EventArgs& /*args*/) {
            StopAutoAdjustTextNumber();
            return true;
            });

        pDownButton->DetachEvent(kEventClick);
        pDownButton->AttachClick([this](const EventArgs& /*args*/) {
            StopAutoAdjustTextNumber();
            AdjustTextNumber(-1);
            return true;
            });

        pDownButton->DetachEvent(kEventMouseButtonDown);
        pDownButton->AttachButtonDown([this](const EventArgs& /*args*/) {
            StartAutoAdjustTextNumberTimer(-1);
            return true;
            });

        pDownButton->DetachEvent(kEventMouseButtonUp);
        pDownButton->AttachButtonUp([this](const EventArgs& /*args*/) {
            StopAutoAdjustTextNumber();
            return true;
            });

        pDownButton->DetachEvent(kEventMouseLeave);
        pDownButton->AttachMouseLeave([this](const EventArgs& /*args*/) {
            StopAutoAdjustTextNumber();
            return true;
            });
        return true;
    }
    else {
        if (m_pSpinBox != nullptr) {
            RemoveItem(m_pSpinBox);
            m_pSpinBox = nullptr;
        }    
    }
    return false;
}

bool RichEdit::SetEnableSpin(bool bEnable, const DString& spinClass, int32_t nMin, int32_t nMax)
{
    bool bRet = false;
    if (bEnable) {
        ASSERT(!spinClass.empty());
        if (spinClass.empty()) {
            return false;
        }
        bRet = SetSpinClass(spinClass);
        if (bRet) {
            SetNumberOnly(true);
            if ((nMin != 0) || (nMax != 0)) {
                SetMaxNumber(nMax);
                SetMinNumber(nMin);
            }            
        }
    }
    else {
        bool hasSpin = m_pSpinBox != nullptr;
        SetSpinClass(_T(""));
        bRet = true;
        if (hasSpin) {
            SetNumberOnly(false);
            SetMaxNumber(INT_MAX);
            SetMinNumber(INT_MIN);
        }        
    }
    return bRet;
}

int64_t RichEdit::GetTextNumber() const
{
    DString text = GetText();
    if (text.empty()) {
        return 0;
    }
    int64_t n = StringUtil::StringToInt64(text);
    return n;
}

void RichEdit::SetTextNumber(int64_t nValue)
{
    SetText(StringUtil::Printf(_T("%I64d"), nValue));
}

void RichEdit::AdjustTextNumber(int32_t nDelta)
{
    ASSERT(IsNumberOnly());
    if (IsNumberOnly()) {
        const int64_t nOldValue = GetTextNumber();
        int64_t nNewValue = nOldValue + nDelta;
        if (((GetMinNumber() != INT_MIN) || (GetMaxNumber() != INT_MAX))) {
            if (nNewValue > GetMaxNumber()) {
                //超过最大数字，进行修正
                nNewValue = GetMaxNumber();
            }
            else if (nNewValue < GetMinNumber()) {
                //小于最小数字，进行修正
                nNewValue = GetMinNumber();
            }
        }
        if (nNewValue != nOldValue) {
            SetTextNumber(nNewValue);
        }
    }
}

void RichEdit::StartAutoAdjustTextNumberTimer(int32_t nDelta)
{
    if (nDelta != 0) {
        //启动定时器
        m_flagAdjustTextNumber.Cancel();
        std::function<void()> closure = UiBind(&RichEdit::StartAutoAdjustTextNumber, this, nDelta);
        GlobalManager::Instance().Timer().AddTimer(m_flagAdjustTextNumber.GetWeakFlag(), closure, 1000, 1);
    }
}

void RichEdit::StartAutoAdjustTextNumber(int32_t nDelta)
{
    if (nDelta != 0) {
        //启动定时器
        m_flagAdjustTextNumber.Cancel();
        std::function<void()> closure = UiBind(&RichEdit::AdjustTextNumber, this, nDelta);
        GlobalManager::Instance().Timer().AddTimer(m_flagAdjustTextNumber.GetWeakFlag(), closure, 120);
    }
}

void RichEdit::StopAutoAdjustTextNumber()
{
    m_flagAdjustTextNumber.Cancel();
}

void RichEdit::SetClearBtnClass(const DString& btnClass)
{
    if (!btnClass.empty()) {
        ASSERT(m_pClearButton == nullptr);
        if (m_pClearButton != nullptr) {
            return;
        }
        Button* pClearButton = new Button(GetWindow());
        pClearButton->SetClass(btnClass);
        pClearButton->SetNoFocus();
        pClearButton->SetVisible(false);
        AddItem(pClearButton);
        m_pClearButton = pClearButton;

        //响应按钮点击事件
        pClearButton->AttachClick([this](const EventArgs& /*args*/) {
            SetText(_T(""));
            return true;
            });
    }
}

void RichEdit::SetShowPasswordBtnClass(const DString& btnClass)
{
    if (!btnClass.empty()) {
        ASSERT(m_pShowPasswordButton == nullptr);
        if (m_pShowPasswordButton != nullptr) {
            return;
        }
        Button* pButton = new Button(GetWindow());
        pButton->SetClass(btnClass);
        pButton->SetNoFocus();
        pButton->SetVisible(false);
        AddItem(pButton);
        m_pShowPasswordButton = pButton;

        //响应按钮点击事件
        pButton->AttachClick([this](const EventArgs& /*args*/) {
            SetShowPassword(false);
            return true;
            });
        pButton->AttachButtonDown([this](const EventArgs& /*args*/) {
            SetShowPassword(true);
            return true;
            });
        pButton->AttachButtonUp([this](const EventArgs& /*args*/) {
            SetShowPassword(false);
            return true;
            });
        pButton->AttachMouseLeave([this](const EventArgs& /*args*/) {
            SetShowPassword(false);
            return true;
            });
    }
    else {
        if (m_pShowPasswordButton != nullptr) {
            RemoveItem(m_pShowPasswordButton);
            m_pShowPasswordButton = nullptr;
        }
    }
}

void RichEdit::SetFontIdInternal(const DString& fontId)
{
    //创建光标
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(fontId, Dpi());
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return;
    }
    IRender* pRender = nullptr;
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        pRender = pWindow->GetRender();
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    //按字体高度设置光标的高度
    UiRect fontRect = pRender->MeasureString(L"T", pFont, 0);
    m_nLineHeight = fontRect.Height();
    ASSERT(m_nLineHeight > 0);
    int32_t nCaretHeight = fontRect.Height();
    if (nCaretHeight > 1) {
        nCaretHeight -= Dpi().GetScaleInt(1);
    }
    else {
        nCaretHeight = Dpi().GetScaleInt(1);
    }
    int32_t nCaretWidth = Dpi().GetScaleInt(1);
    CreateCaret(nCaretWidth, nCaretHeight);
}

void RichEdit::SetHAlignType(HorAlignType alignType)
{
    if (m_hAlignType != alignType) {
        m_hAlignType = alignType;
        Redraw();
    }
}

HorAlignType RichEdit::GetHAlignType() const
{
    return m_hAlignType;
}

void RichEdit::SetVAlignType(VerAlignType alignType)
{
    if (m_vAlignType != alignType) {
        m_vAlignType = alignType;
        Redraw();
    }
}

VerAlignType RichEdit::GetVAlignType() const
{
    return m_vAlignType;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

// 单行模式：忽略回车和换行【绘制函数：完成】
// 自动换行模式【绘制函数：完成】
// 支持TAB键的绘制: 按4个字符的倍数对齐，输入的时候替换，绘制的时候不含TAB键
// 坐标相关：
// （1）根据一个点返回光标的位置
// （2）int32_t GetSel/SetSel(int32_t nStartChar, int32_t nEndChar);【半个字符的处理】
// （3）GetTextRange(int32_t nStartChar, int32_t nEndChar)
// （4）HideSelection(bool bHideSelection
// （5）行相关：GetLineCount，
//            DString GetLine(int32_t nIndex, int32_t nMaxLength) const;
//            int32_t LineIndex(int32_t nLine = -1) const;
//            int32_t LineLength(int32_t nLine = -1) const;
//            bool LineScroll(int32_t nLines);
//            int32_t LineFromChar(int32_t nIndex) const;
//            UiPoint PosFromChar(int32_t nChar) const;
//            int32_t CharFromPos(UiPoint pt) const;
//
void RichEdit::AdjustCaretPos(const UiPoint& pt)
{
    UiPoint cursorPos;
    if ((pt.x < 0) && (pt.y < 0)) {
        Window* pWindow = GetWindow();
        if (pWindow != nullptr) {
            pWindow->GetCursorPos(cursorPos);
            pWindow->ScreenToClient(cursorPos);
        }        
    }
    else {
        cursorPos = pt;
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    rc.Deflate(GetTextPadding());
    cursorPos.Offset(GetScrollOffsetInScrollBox());

    //光标：横向按字符边界对齐，纵向按行高对齐
    bool bSet = false;
    if (!rc.ContainsPt(cursorPos)) {
        cursorPos.x = rc.left;
        cursorPos.y = rc.top;
        bSet = true;
    }
    else if (!m_textRects.empty()) {
        for (const MeasureCharRects& charInfo : m_textRects) {
            if (charInfo.m_charRect.ContainsPt((float)cursorPos.x, (float)cursorPos.y)) {
                if (cursorPos.x < charInfo.m_charRect.CenterX()) {
                    cursorPos.x = charInfo.m_charRect.left;
                }
                else {
                    cursorPos.x = charInfo.m_charRect.right;
                }                
                cursorPos.y = charInfo.m_charRect.top;
                bSet = true;
                break;
            }
        }
        if (!bSet) {
            //检测行数据信息
            uint32_t nLineNumber = (uint32_t)-1;
            auto iterFirst = m_lineTextRects.begin();            
            if (iterFirst != m_lineTextRects.end()) {
                if (cursorPos.y < iterFirst->second.top) {
                    cursorPos.y = iterFirst->second.top;
                    nLineNumber = iterFirst->first;
                }
                else {
                    auto iterLast = m_lineTextRects.rbegin();
                    while (iterLast != m_lineTextRects.rend()) {
                        if (cursorPos.y >= iterLast->second.top) {
                            cursorPos.y = iterLast->second.top;
                            nLineNumber = iterLast->first;
                            break;
                        }
                        ++iterLast;
                    }
                }
            }
            auto iter = m_lineTextRects.find(nLineNumber);
            if (iter != m_lineTextRects.end()) {
                if (cursorPos.x <= iter->second.left) {
                    cursorPos.x = iter->second.left;
                }
                else if (cursorPos.x > iter->second.right) {
                    cursorPos.x = iter->second.right;
                }
                else {
                    cursorPos.x = iter->second.right;
                }
                bSet = true;
            }
        }
    }
    if (!bSet) {
        cursorPos.x = rc.left;
        cursorPos.y = rc.top;
    }
    SetCaretPos(cursorPos.x, cursorPos.y);
}

uint32_t RichEdit::GetTextStyle() const
{
    uint32_t uTextStyle = 0;
    if (m_hAlignType == HorAlignType::kHorAlignCenter) {
        uTextStyle |= TEXT_CENTER;
    }
    else if (m_hAlignType == HorAlignType::kHorAlignRight) {
        uTextStyle |= TEXT_RIGHT;
    }
    else {
        uTextStyle |= TEXT_LEFT;
    }

    if (m_vAlignType == VerAlignType::kVerAlignCenter) {
        uTextStyle |= TEXT_VCENTER;
    }
    else if (m_vAlignType == VerAlignType::kVerAlignBottom) {
        uTextStyle |= TEXT_BOTTOM;
    }
    else {
        uTextStyle |= TEXT_TOP;
    }

    if (IsWordWrap()) {
        uTextStyle |= TEXT_WORD_WRAP;
    }
    else {
        uTextStyle &= ~TEXT_WORD_WRAP;
    }

    //单行/多行属性
    if (IsMultiLine()) {
        uTextStyle &= ~TEXT_SINGLELINE;
    }
    else {
        uTextStyle |= TEXT_SINGLELINE;
    }
    return uTextStyle;
}

bool RichEdit::GetRichTextForDraw(std::vector<RichTextData>& richTextDataList) const
{
    if (m_text.empty()) {
        return false;
    }
    DString sFontId = GetFontId();
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(sFontId, Dpi());
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return false;
    }
    RichTextData richTextData;
    richTextData.m_textView = m_text;
    //默认文本属性
    richTextData.m_uTextStyle = GetTextStyle();
    //默认文本颜色
    richTextData.m_textColor = GetUiColor(GetTextColor());
    if (richTextData.m_textColor.IsEmpty()) {
        richTextData.m_textColor = UiColor(UiColors::Black);
    }
    //文本的字体信息
    richTextData.m_fontInfo.m_fontName = pFont->FontName();
    richTextData.m_fontInfo.m_fontSize = pFont->FontSize();
    richTextData.m_fontInfo.m_bBold = pFont->IsBold();
    richTextData.m_fontInfo.m_bUnderline = pFont->IsUnderline();
    richTextData.m_fontInfo.m_bItalic = pFont->IsItalic();
    richTextData.m_fontInfo.m_bStrikeOut = pFont->IsStrikeOut();

    richTextDataList.emplace_back(std::move(richTextData));
    return true;
}

void RichEdit::CalcDestRect(IRender* pRender, const UiRect& rc, UiRect& rect) const
{
    rect.Clear();
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }

    std::vector<RichTextData> richTextDataList;
    if (GetRichTextForDraw(richTextDataList)) {
        pRender->MeasureRichText(rc, pRenderFactory, richTextDataList);
        for (const RichTextData& data : richTextDataList) {
            for (const UiRect& textRect : data.m_textRects) {
                rect.Union(textRect);
            }
        }
    }
}

void RichEdit::CalcTextRects()
{
    m_textRect.Clear();
    m_textRects.clear();
    m_lineTextRects.clear();

    IRender* pRender = nullptr;
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        pRender = pWindow->GetRender();
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }

    std::vector<RichTextData> richTextDataList;
    if (!GetRichTextForDraw(richTextDataList)) {
        return;
    }

    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    rc.Deflate(GetTextPadding());

    pRender->MeasureRichText(rc, pRenderFactory, richTextDataList, &m_textRects);
    ASSERT(m_textRects.size() == m_text.size());
    if (!m_textRects.empty()) {
        //计算每行的矩形区域
        for (const MeasureCharRects& charInfo : m_textRects) {
            if (charInfo.m_bReturn || charInfo.m_bNewLine || charInfo.m_bLowSurrogate) {
                continue;
            }
            else {
                m_lineTextRects[charInfo.m_nRowIndex].Union(charInfo.m_charRect);
            }
        }
        for (const MeasureCharRects& charInfo : m_textRects) {
            if (charInfo.m_bNewLine) {
                if (m_lineTextRects.find(charInfo.m_nRowIndex) == m_lineTextRects.end()) {
                    //本行只有一个回车
                    UiRectF& rect = m_lineTextRects[charInfo.m_nRowIndex];
                    rect.left = rc.left;
                    rect.right = rc.left;

                    auto iterPrev = m_lineTextRects.find(charInfo.m_nRowIndex - 1);
                    if (iterPrev != m_lineTextRects.end()) {
                        //中间行
                        rect.top = iterPrev->second.bottom;
                    }
                    else {
                        //首行 
                        rect.top = rc.top;
                    }
                    auto iterNext = m_lineTextRects.find(charInfo.m_nRowIndex + 1);
                    if (iterNext != m_lineTextRects.end()) {
                        //中间行
                        rect.bottom = iterNext->second.top;
                    }
                    else {
                        //尾行
                        rect.bottom = rect.top + m_nLineHeight;
                    }
                }
            }
        }
        UiRectF textRect;
        for (const auto& iter : m_lineTextRects) {
            textRect.Union(iter.second);
        }
        m_textRect.left = (int32_t)textRect.left;
        m_textRect.top = (int32_t)textRect.top;
        m_textRect.right = (int32_t)textRect.right;
        m_textRect.bottom = (int32_t)textRect.bottom;
        if ((textRect.right - m_textRect.right) > 0.01f) {
            m_textRect.right += 1;
        }
        if ((textRect.bottom - m_textRect.bottom) > 0.01f) {
            m_textRect.bottom += 1;
        }
    }
}

UiSize RichEdit::EstimateText(UiSize szAvailable)
{
    UiSize fixedSize;
    IRender* pRender = nullptr;
    if (GetWindow() != nullptr) {
        pRender = GetWindow()->GetRender();
    }
    if (pRender == nullptr) {
        return fixedSize;
    }

    int32_t nWidth = szAvailable.cx;
    if (GetFixedWidth().IsStretch()) {
        //如果是拉伸类型，使用外部宽度
        nWidth = CalcStretchValue(GetFixedWidth(), szAvailable.cx);
    }
    else if (GetFixedWidth().IsInt32()) {
        nWidth = GetFixedWidth().GetInt32();
    }
    else if (GetFixedWidth().IsAuto()) {
        //宽度为自动时，不限制宽度
        nWidth = INT_MAX;
    }

    //最大高度，不限制
    int32_t nHeight = INT_MAX;

    UiRect rc;
    rc.left = 0;
    rc.right = rc.left + nWidth;
    rc.top = 0;
    rc.bottom = rc.top + nHeight;

    const UiPadding rcTextPadding = GetTextPadding();
    const UiPadding rcPadding = GetControlPadding();
    if (nWidth != INT_MAX) {
        rc.left += (rcPadding.left + rcTextPadding.left);
        rc.right -= (rcPadding.right + rcTextPadding.right);
    }
    if (nHeight != INT_MAX) {
        rc.top += (rcPadding.top + rcTextPadding.top);
        rc.bottom -= (rcPadding.bottom + rcTextPadding.bottom);
    }
    if (rc.IsEmpty()) {
        return fixedSize;
    }

    //计算绘制所占的区域大小
    UiRect rect;
    CalcDestRect(pRender, rc, rect);

    fixedSize.cx = rect.Width();
    if (fixedSize.cx > 0) {
        fixedSize.cx += (rcTextPadding.left + rcTextPadding.right);
        fixedSize.cx += (rcPadding.left + rcPadding.right);
    }

    fixedSize.cy = rect.Height();
    if (fixedSize.cy > 0) {
        fixedSize.cy += (rcTextPadding.top + rcTextPadding.bottom);
        fixedSize.cy += (rcPadding.top + rcPadding.bottom);
    }
    return fixedSize;
}

UiSize64 RichEdit::CalcRequiredSize(const UiRect& rc)
{
    //计算子控件的大小
    UiSize64 requiredSize = __super::CalcRequiredSize(rc);
    if (requiredSize.cx > rc.Width()) {
        requiredSize.cx = 0;
    }
    else if (requiredSize.cy > rc.Height()) {
        requiredSize.cy = 0;
    }

    UiRect rcAvailable = rc;
    rcAvailable.Deflate(GetTextPadding());
    rcAvailable.Deflate(GetControlPadding());
    UiSize szAvailable(rcAvailable.Width(), rcAvailable.Height());

    //估算图片区域大小
    UiSize imageSize = EstimateImage(szAvailable);
    if (imageSize.cx > rc.Width()) {
        imageSize.cx = 0;
    }
    else if (imageSize.cy > rc.Height()) {
        imageSize.cy = 0;
    }

    //估算文本区域大小, 函数计算时，已经包含了内边距
    UiSize textSize = EstimateText(szAvailable);

    UiSize szControlSize;
    szControlSize.cx = std::max(imageSize.cx, textSize.cx);
    szControlSize.cy = std::max(imageSize.cy, textSize.cy);

    //以文本的大小为准，子控件或者背景图的大小，不影响控件是否出现滚动条
    UiSize64 szSize = requiredSize;
    if (szSize.cx < szControlSize.cx) {
        szSize.cx = szControlSize.cx;
    }
    if (szSize.cy < szControlSize.cy) {
        szSize.cy = szControlSize.cy;
    }
    return szSize;
}

void RichEdit::Redraw()
{

    Invalidate();
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
