#include "DateTime.h"
#include <windowsx.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <olectl.h>
#include <CommCtrl.h>

namespace ui
{

class DateTimeWnd : public Window
{
public:
    DateTimeWnd();
    virtual ~DateTimeWnd();

    void Init(DateTime* pOwner);
    HFONT CreateHFont() const;
    UiRect CalPos();

    virtual std::wstring GetWindowClassName() const override;
    virtual std::wstring GetSuperClassName() const override;
    void OnFinalMessage(HWND hWnd);

    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
protected:
    DateTime* m_pOwner;
    bool m_bInit;
    bool m_bDropOpen;
    SYSTEMTIME m_oldSysTime;
    HFONT m_hFont;
};

DateTimeWnd::DateTimeWnd() : 
    m_pOwner(nullptr), 
    m_bInit(false), 
    m_bDropOpen(false),
    m_oldSysTime({0,}),
    m_hFont(nullptr)
{
}

DateTimeWnd::~DateTimeWnd()
{
    if (m_hFont != nullptr) {
        ::DeleteObject(m_hFont);
        m_hFont = nullptr;
    }
}

void DateTimeWnd::Init(DateTime* pOwner)
{
    m_pOwner = pOwner;
    if (GetHWND() == nullptr) {
        UiRect rcPos = CalPos();
        UINT uStyle = WS_POPUP;
        UiPoint pt1 = { rcPos.left, rcPos.top };
        UiPoint pt2 = { rcPos.right, rcPos.bottom };
        ClientToScreen(pOwner->GetWindow()->GetHWND(), pt1);
        ClientToScreen(pOwner->GetWindow()->GetHWND(), pt2);
        CreateWnd(m_pOwner->GetWindow()->GetHWND(), L"", uStyle, 0, {pt1.x, pt1.y, pt2.x, pt2.y});
        ASSERT(GetHWND() != nullptr);

        HFONT hFont = CreateHFont();
        if (hFont != nullptr) {
            if (m_hFont != nullptr) {
                ::DeleteObject(m_hFont);
                m_hFont = nullptr;
            }
            m_hFont = hFont;
        }
        SetWindowFont(GetHWND(), m_hFont, TRUE);
    }

    if (m_pOwner->IsValidTime()) {
        memcpy(&m_oldSysTime, &m_pOwner->m_sysTime, sizeof(SYSTEMTIME));
    }
    else
    {
        ::GetLocalTime(&m_oldSysTime);
    }

    ::SendMessage(GetHWND(), DTM_SETSYSTEMTIME, 0, (LPARAM)&m_oldSysTime);
    ::ShowWindow(GetHWND(), SW_SHOW);
    ::SetFocus(GetHWND());

    HWND hWndParent = GetHWND();
    while (::GetParent(hWndParent) != NULL) {
        hWndParent = ::GetParent(hWndParent);
    }        
    ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
    m_bInit = true;
}

HFONT DateTimeWnd::CreateHFont() const
{
    //优先获取默认字体
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(L"");
    if (pFont == nullptr) {
        pFont = GlobalManager::Instance().Font().GetIFont(L"system_12");
    }
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return nullptr;
    }
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    wcscpy_s(lf.lfFaceName, pFont->FontName());
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = -pFont->FontSize();
    if (pFont->IsUnderline()) {
        lf.lfUnderline = TRUE;
    }
    if (pFont->IsStrikeOut()) {
        lf.lfStrikeOut = TRUE;
    }
    if (pFont->IsItalic()) {
        lf.lfItalic = TRUE;
    }
    if (pFont->IsBold()) {
        lf.lfWeight = FW_BOLD;
    }
    HFONT hFont = ::CreateFontIndirect(&lf);
    ASSERT(hFont != nullptr);
    return hFont;
}

UiRect DateTimeWnd::CalPos()
{
    UiRect rcPos = m_pOwner->GetPos();
    Control* pParent = m_pOwner->GetParent();
    UiRect rcParent;
    while (pParent != nullptr) {
        if (!pParent->IsVisible()) {
            rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
            break;
        }
        rcParent = pParent->GetPos();
        if (!UiRect::Intersect(rcPos, rcPos, rcParent)) {
            rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
            break;
        }
        pParent = pParent->GetParent();
    }
    return rcPos;
}

std::wstring DateTimeWnd::GetWindowClassName() const
{
    return L"DateTimeWnd";
}

std::wstring DateTimeWnd::GetSuperClassName() const
{
    return DATETIMEPICK_CLASS;
}

void DateTimeWnd::OnFinalMessage(HWND /*hWnd*/)
{
    m_pOwner->m_pDateWindow = nullptr;
    delete this;
}

LRESULT DateTimeWnd::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lRes = 0;
    bHandled = false;
    if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        m_pOwner->SetTime(m_oldSysTime);
        PostMessage(WM_CLOSE);
        bHandled = true;
    }
    else if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
        SYSTEMTIME systime = { 0, };
        ::SendMessage(GetHWND(), DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
        m_pOwner->SetTime(systime);
        PostMessage(WM_CLOSE);
        bHandled = true;
    }
    else if (uMsg == OCM_NOTIFY)
    {
        NMHDR* pHeader = (NMHDR*)lParam;
        if (pHeader != nullptr && pHeader->hwndFrom == GetHWND()) {
            if (pHeader->code == DTN_DATETIMECHANGE) {
                SYSTEMTIME systime = {0,};
                ::SendMessage(GetHWND(), DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
                m_pOwner->SetTime(systime);
            }
            else if (pHeader->code == DTN_DROPDOWN) {
                m_bDropOpen = true;
            }
            else if (pHeader->code == DTN_CLOSEUP) {
                SYSTEMTIME systime = { 0, };
                ::SendMessage(GetHWND(), DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
                m_pOwner->SetTime(systime);
                PostMessage(WM_CLOSE);
                m_bDropOpen = false;
            }
        }
    }
    else if (uMsg == WM_KILLFOCUS)
    {
        if (!m_bDropOpen) {
            PostMessage(WM_CLOSE);
        }
    }
    if (!bHandled) {
        bHandled = true;
        lRes = this->CallDefaultWindowProc(uMsg, wParam, lParam);
    }
    return lRes;
}

//////////////////////////////////////////////////////////////////////////

DateTime::DateTime():
    m_sysTime({0,}),
    m_bReadOnly(false),
    m_pDateWindow(nullptr)
{
    m_sFormat = std::wstring(L"%Y-%m-%d");
    //设置默认属性
    SetAttribute(L"border_size", L"1");
    SetAttribute(L"border_color", L"gray");
    SetAttribute(L"text_align", L"vcenter");
    SetAttribute(L"text_padding", L"2,0,0,0");
}

DateTime::~DateTime()
{
}

std::wstring DateTime::GetType() const { return DUI_CTR_DATETIME; }

void DateTime::InitLocalTime()
{
    SYSTEMTIME st = {0,};
    ::GetLocalTime(&st);
    SetTime(st);
}

void DateTime::ClearTime()
{
    SYSTEMTIME st = {0,};
    SetTime(st);
}

const SYSTEMTIME& DateTime::GetSystemTime() const
{
    return m_sysTime;
}

std::wstring DateTime::GetDateTime() const
{
    std::wstring dateTime;
    if (IsValidTime()) {
        std::tm tmSystemDate = { 0, };
        tmSystemDate.tm_sec = m_sysTime.wSecond;
        tmSystemDate.tm_min = m_sysTime.wMinute;
        tmSystemDate.tm_hour = m_sysTime.wHour;
        tmSystemDate.tm_mday = m_sysTime.wDay;
        tmSystemDate.tm_mon = m_sysTime.wMonth - 1;
        tmSystemDate.tm_year = m_sysTime.wYear - 1900;
        tmSystemDate.tm_isdst = -1;

        std::wstringstream ss;
        ss << std::put_time(&tmSystemDate, m_sFormat.c_str());
        dateTime = ss.str();
    }
    return dateTime;
}

void DateTime::SetTime(const SYSTEMTIME& systemTime)
{
    if (IsEqual(systemTime)) {
        return;
    }
    m_sysTime = systemTime;    
    SetText(GetDateTime());
    Invalidate();

    ui::EventArgs args;
    args.pSender = this;
    args.Type = kEventValueChange;
    FireAllEvents(args);
}

bool DateTime::IsEqual(const SYSTEMTIME& st) const
{
    if (m_sysTime.wYear == st.wYear &&
        m_sysTime.wMonth == st.wMonth &&
        m_sysTime.wDay == st.wDay &&
        m_sysTime.wHour == st.wHour &&
        m_sysTime.wMinute == st.wMinute &&
        m_sysTime.wSecond == st.wSecond &&
        m_sysTime.wMilliseconds == st.wMilliseconds) {
        return true;
    }
    return false;
}

void DateTime::SetReadOnly(bool bReadOnly)
{
    m_bReadOnly = bReadOnly;
    Invalidate();
}

bool DateTime::IsReadOnly() const
{
    return m_bReadOnly;
}

bool DateTime::IsValidTime() const
{
    if (m_sysTime.wYear == 0 && 
        m_sysTime.wMonth == 0 && 
        m_sysTime.wDay == 0 && 
        m_sysTime.wHour == 0 && 
        m_sysTime.wMinute == 0 && 
        m_sysTime.wSecond == 0 && 
        m_sysTime.wMilliseconds == 0) {
        return false;
    }
    return true;
}

void DateTime::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"format") {
        SetFormat(strValue);
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

std::wstring DateTime::GetFormat() const
{
    return m_sFormat.c_str();
}

void DateTime::SetFormat(const std::wstring& val)
{
    if (m_sFormat == val) {
        return;
    }
    m_sFormat = val;
    SetText(GetDateTime());
    Invalidate();
}

void DateTime::HandleEvent(const EventArgs& event)
{
    if (!IsMouseEnabled() && 
        (event.Type > kEventMouseBegin) && 
        (event.Type < kEventMouseEnd)) {
        if (GetParent() != nullptr) {
            GetParent()->SendEvent(event);
        }
        else {
            __super::HandleEvent(event);
        }
        return;
    }

    if ((event.Type == kEventSetCursor) && IsEnabled()) {
        ::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
        return;
    }
    if (event.Type == kEventWindowSize) {
        if (m_pDateWindow != nullptr) {
            return;
        }
    }
    if (event.Type == kEventScrollChange) {
        if (m_pDateWindow != nullptr) {
            return;
        }
    }
    if (event.Type == kEventSetFocus && IsEnabled()) {
        if (m_pDateWindow != nullptr) {
            return;
        }
        m_pDateWindow = new DateTimeWnd();
        m_pDateWindow->Init(this);
        m_pDateWindow->ShowWindow();
    }
    if (event.Type == kEventKillFocus && IsEnabled()) {
        Invalidate();
    }
    if ((event.Type == kEventMouseButtonDown) || 
        (event.Type == kEventMouseDoubleClick) ||
        (event.Type == kEventMouseRightButtonDown)) {
        if (IsEnabled()) {
            if (GetWindow() != nullptr) {
                GetWindow()->ReleaseCapture();
            }            
            if (IsFocused() && (m_pDateWindow == nullptr)) {
                m_pDateWindow = new DateTimeWnd();
            }
            if (m_pDateWindow != nullptr) {
                m_pDateWindow->Init(this);
                m_pDateWindow->ShowWindow();
            }
        }
        return;
    }
    if (event.Type == kEventMouseMove) {
        return;
    }
    if (event.Type == kEventMouseButtonUp) {
        return;
    }
    if (event.Type == kEventMouseMenu) {
        return;
    }
    if (event.Type == kEventMouseEnter) {
        return;
    }
    if (event.Type == kEventMouseLeave) {
        return;
    }
    __super::HandleEvent(event);
}

}//namespace ui
