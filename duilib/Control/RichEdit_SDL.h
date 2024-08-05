#ifndef UI_CONTROL_RICHEDIT_SDL_H_
#define UI_CONTROL_RICHEDIT_SDL_H_

#include "duilib/Box/ScrollBox.h"
#include "duilib/Image/Image.h"

#ifdef DUILIB_BUILD_FOR_SDL

namespace ui 
{

class VBox;
class UILIB_API RichEdit : public ScrollBox
{
public:
    explicit RichEdit(Window* pWindow);
    RichEdit(const RichEdit& r) = delete;
    RichEdit& operator=(const RichEdit& r) = delete;
    virtual ~RichEdit() override;
public:
    //基类的虚函数重写
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& pstrName, const DString& pstrValue) override;
    virtual void SetEnabled(bool bEnable = true) override;
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;
    virtual void SetScrollPos(UiSize64 szPos) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void ClearImageCache() override;
    virtual void HandleEvent(const EventArgs& msg) override;
    virtual UiSize EstimateText(UiSize szAvailable) override;
    virtual UiSize64 CalcRequiredSize(const UiRect& rc) override;

public:
    /** 设置控件的文本, 会触发文本变化事件
     * @param [in] strText 要设置的文本内容
     */
    void SetText(const DString& strText);

    /** 设置控件的文本，不触发文本变化事件
     * @param [in] strText 要设置的文本内容
     */
    void SetTextNoEvent(const DString& strText);

    /** 设置控件的文本对应 ID
     * @param[in] strTextId 要设置的 ID，该 ID 必须在加载的语言文件中存在
     */
    void SetTextId(const DString& strTextId);

    /** 获取控件中的文本
     * @return 返回控件中的文本内容
     */
    DString GetText() const;

    /** 获取内容的长度(按UTF16编码的字符个数，TODO：有歧义)
     * @return 返回内容长度
     */
    int32_t GetTextLength() const;

    /** 获取控件中的文本
     * @return 返回控件中的文本内容 UTF8 格式
     */
    virtual std::string GetUTF8Text() const;

    /** 设置控件的文本对应 ID
     * @param[in] strTextId 要设置的 UTF8 格式 ID，该 ID 必须在加载的语言文件中存在
     */
    virtual void SetUTF8Text(const std::string& strText);

    /** 获取当前设置的字体索引
     * @return 返回字体索引（对应 global.xml 中字体的顺序）
     */
    DString GetFontId() const;

    /** 设置字体索引
     * @param[in] index 要设置的字体索引（对应 global.xml 中字体的顺序）
     */
    void SetFontId(const DString& strFontId);

    /** 设置正常文本颜色
     * @param[in] dwTextColor 要设置的文本颜色，该颜色可在 global.xml 中存在
     */
    void SetTextColor(const DString& dwTextColor);

    /** 获取正常文本颜色
     */
    DString GetTextColor() const;

    /** 设置Disabled状态的文本颜色
     * @param[in] dwTextColor 要设置的文本颜色，该颜色可在 global.xml 中存在
     */
    void SetDisabledTextColor(const DString& dwTextColor);

    /** 获取Disabled状态的文本颜色
     */
    DString GetDisabledTextColor() const;

public:
    /** 设置是否显示提示文字
 * @param[in] bPrompt 设置为 true 为显示，false 为不显示
 */
    void SetPromptMode(bool bPrompt);

    /** 获取提示文字
     */
    DString GetPromptText() const;

    /** 获取提示文字
     * @return 返回 UTF8 格式的提示文字
     */
    std::string GetUTF8PromptText() const;

    /** 设置提示文字
     * @param[in] strText 要设置的提示文字
     */
    void SetPromptText(const DString& strText);

    /** 设置提示文字
     * @param[in] strText 要设置的 UTF8 格式提示文字
     */
    void SetUTF8PromptText(const std::string& strText);

    /** 设置提示文字 ID
     * @param[in] strText 要设置的提示文字 ID，该 ID 必须在加载的语言文件中存在
     */
    void SetPromptTextId(const DString& strTextId);

    /** 设置提示文字 ID
     * @param[in] strText 要设置的 UTF8 格式提示文字 ID，该 ID 必须在加载的语言文件中存在
     */
    void SetUTF8PromptTextId(const std::string& strTextId);

public:
    /** 设置文字内边距信息
     * @param[in] padding 内边距信息
     * @param[in] bNeedDpiScale 是否支持DPI缩放
     */
    void SetTextPadding(UiPadding padding, bool bNeedDpiScale);

    /** 获取文字内边距
     */
    UiPadding GetTextPadding() const;

    /** 是否为多行文本
    */
    bool IsMultiLine() const;

    /** 设置是否为多行文本
    */
    void SetMultiLine(bool bMultiLine);

    /** 获取超出矩形区域的文本显示方式
     * @return 返回 true 时并且在多行模式下内容被换行显示，false 则表示截断显示
     */
    bool IsWordWrap() const;

    /** 设置超出矩形区域的文本显示方式
     * @param[in] bWordWrap 为 true 时并且在多行模式下内容被换行显示，false 则表示截断显示
     */
    void SetWordWrap(bool bWordWrap);

    /** 判断是否接受 TAB 按键消息
     * @return 返回 true 表示接受，TAB键会作为输入字符转换为文本；false 表示不接受，TAB键会作为控件的快捷键，而不作为输入文本字符
     */
    virtual bool IsWantTab() const override;

    /** 判断是否接受 TAB 按键消息
     * @param[in] bWantTab 为 true 表示接受，TAB键会作为输入字符转换为文本；false 表示不接受，TAB键会作为控件的快捷键，而不作为输入文本字符
     */
    void SetWantTab(bool bWantTab);

    /** 判断是否接受 回车键 按键消息
     * @return 返回 true 表示接受，回车键会作为输入字符转换为文本；false 表示不接受，回车键会作为控件的快捷键，而不作为输入文本字符
     */
    bool IsWantReturn() const;

    /** 设置是否接受 回车键 按键消息
     * @param[in] bWantReturn 为 true 表示接受，回车键会作为输入字符转换为文本；false 表示不接受，回车键会作为控件的快捷键，而不作为输入文本字符
     */
    void SetWantReturn(bool bWantReturn);

    /** 判断是否接受CTRL+RETURN 组合键消息
     * @return 返回 true 表示接受，false 表示不接受
     */
    bool IsWantCtrlReturn() const;

    /** 设置是否接受CTRL+RETUREN 组合键消息
     * @param[in] bWantCtrlReturn 为 true 则接受该消息，false 为不接受
     */
    void SetWantCtrlReturn(bool bWantCtrlReturn);

    /** 是否是只读状态
     * @return 返回 true 为只读状态，否则为 false
     */
    bool IsReadOnly() const;

    /** 设置控件为只读状态
     * @param[in] bReadOnly 设置 true 让控件变为只读状态，false 为可写入状态
     */
    void SetReadOnly(bool bReadOnly);

    /** 是否是密码状态控件
     * @return 返回 true 表示是密码控件，否则为 false
     */
    bool IsPassword() const;

    /** 设置控件为密码控件（显示 ***）
     * @param[in] bPassword 设置为 true 让控件显示内容为 ***，false 为显示正常内容
     */
    void SetPassword(bool bPassword);

    /** 设置是否显示密码
    */
    void SetShowPassword(bool bShow);

    /** 是否显示密码
    */
    bool IsShowPassword() const;

    /** 设置密码字符
    */
    void SetPasswordChar(wchar_t ch);

    /** 设置是否对输入的字符短暂显示再隐藏（仅当IsShowPassword()为true，即密码模式的时候有效）
    */
    void SetFlashPasswordChar(bool bFlash);

    /** 获取是否对输入的字符短暂显示再隐藏
    */
    bool IsFlashPasswordChar() const;

    /** 是否只允许输入数字
    */
    bool IsNumberOnly() const;

    /** 设置是否只允许输入数字
    */
    void SetNumberOnly(bool bNumberOnly);

    /** 设置允许的最大数字(仅当IsNumberOnly()为true的时候有效)
    */
    void SetMaxNumber(int32_t maxNumber);

    /** 获取允许的最大数字
    */
    int32_t GetMaxNumber() const;

    /** 设置允许的最小数字(仅当IsNumberOnly()为true的时候有效)
    */
    void SetMinNumber(int32_t minNumber);

    /** 获取允许的最小数字
    */
    int32_t GetMinNumber() const;

    /** 获取限制字符数量
     * @return 返回限制字符数量
     */
    int32_t GetLimitText() const;

    /** 设置限制字符数量
     * @param [in] iChars 要限制的字符数量
     */
    void SetLimitText(int32_t iChars);

    /** 获取允许输入哪些字符
    */
    DString GetLimitChars() const;

    /** 设置允许输入哪些字符，比如颜色值可以设置：limit_chars="#0123456789ABCDEFabcdef"
    * @param [in] limitChars 允许输入的字符列表
    */
    void SetLimitChars(const DString& limitChars);

    /** 获取焦点状态下的图片
 * @return 返回焦点状态下的图片
 */
    DString GetFocusedImage();

    /** 设置焦点状态下的图片
     * @param[in] strImage 要设置的图片位置
     */
    void SetFocusedImage(const DString& strImage);

    /** 设置是否允许通过Ctrl + 滚轮来调整缩放比例
    */
    void SetEnableWheelZoom(bool bEnable);

    /** 获取是否允许通过Ctrl + 滚轮来调整缩放比例
    */
    bool IsEnableWheelZoom(void) const;

    /** 是否允许使用默认的右键菜单
    */
    void SetEnableDefaultContextMenu(bool bEnable);

    /** 是否允许使用默认的右键菜单
    */
    bool IsEnableDefaultContextMenu() const;

    /** 设置是否支持Spin控件
    * @param [in] bEnable true表示支持Spin控件，false表示不支持Spin控件
    * @param [in] spinClass Spin控件的Class属性，字符串需要包含3个值，具体设置参见：global.xml里面的rich_edit_spin设置
    *             取值举例：rich_edit_spin_box,rich_edit_spin_btn_up,rich_edit_spin_btn_down
    * @param [in] nMin 表示设置数字的最小值
    * @param [in] nMax 表示设置数字的最大值，如果 nMin和nMax同时为0, 表示不设置数字的最小值和最大值
    */
    bool SetEnableSpin(bool bEnable, const DString& spinClass, int32_t nMin = 0, int32_t nMax = 0);

public:
    /** 创建光标
     * @param [in] xWidth 光标宽度
     * @param [in] yHeight 光标高度
     * @return 成功返回 true，失败返回 false
     */
    bool CreateCaret(int32_t xWidth, int32_t yHeight);

    /** 设置是否显示光标
     * @param [in] fShow 设置 true 为显示，false 为不显示
     * @return 成功返回 true，失败返回 false
     */
    bool ShowCaret(bool fShow);

    /** 设置光标颜色
     * @param[in] dwColor 要设置的颜色值，该值必须在 global.xml 中存在
     * @return 无
     */
    void SetCaretColor(const DString& dwColor);

    /** 获取光标颜色
     * @return 返回光标颜色
     */
    DString GetCaretColor();

    /** 获取光标矩形位置
     * @return 返回光标矩形位置
     */
    UiRect GetCaretRect();

    /** 设置光标位置
     * @param [in] x X 轴坐标
     * @param [in] y Y 轴坐标
     * @return 成功返回 true，失败返回 false
     */
    bool SetCaretPos(int32_t x, int32_t y);

    /** 设置只读模式不显示光标
    */
    void SetNoCaretReadonly();

    /** 设置是否使用Control的光标
    */
    void SetUseControlCursor(bool bUseControlCursor);

public:
    /** 监听回车按键按下事件
     * @param[in] callback 回车被按下的自定义回调函数
     */
    void AttachReturn(const EventCallback& callback) { AttachEvent(kEventReturn, callback); }

    /** 监听 TAB 按键按下事件
     * @param[in] callback TAB 被按下的自定义回调函数
     */
    void AttachTab(const EventCallback& callback) { AttachEvent(kEventTab, callback); }

    /* 监听缩放比例变化事件
     * @param[in] callback 文本被修改后的自定义回调函数
     */
    void AttachZoom(const EventCallback& callback) { AttachEvent(kEventZoom, callback); }

    /* 监听文本被修改事件
     * @param[in] callback 文本被修改后的自定义回调函数
     */
    void AttachTextChange(const EventCallback& callback) { AttachEvent(kEventTextChange, callback); }

    /* 监听文本选择变化事件
     * @param[in] callback 文本选择变化后的自定义回调函数
     */
    void AttachSelChange(const EventCallback& callback);

    /** 监听超级链接被点击事件
     * @param[in] callback 超级链接被点击后的回调函数
     */
    void AttachLinkClick(const EventCallback& callback) { AttachEvent(kEventLinkClick, callback); }

public:
    /** 获取修改标志
     * @return 返回 true 为设置了修改标志，否则为 false
     */
    bool GetModify() const;

    /** 设置修改标志
     * @param[in] bModified 设置为 true 表示文本已经被修改，false 为未修改，默认为 true
     */
    void SetModify(bool bModified = true);

    /** 全选
     * @return 返回选择的内容数量
     */
    int32_t SetSelAll();

    /** 不选择任何内容
     */
    void SetSelNone();

    /** 设置失去焦点后是否取消选择项
     * @param[in] bOnSel 设置为 true 表示取消选择项，false 为不取消
     */
    void SetNoSelOnKillFocus(bool bOnSel);

    /** 设置获取焦点后是否选择所有内容
     * @param[in] bSelAll 设置 true 表示在获取焦点时选择所有内容，false 为不选择
     */
    void SetSelAllOnFocus(bool bSelAll);

    /** 获取所选文本的起始位置和结束位置
     * @param[in] nStartChar 返回起始位置
     * @param[in] nEndChar 返回结束位置
     */
    void GetSel(int32_t& nStartChar, int32_t& nEndChar) const;

    /** 选择一部分内容
     * @param[in] nStartChar 要选择的起始位置
     * @param[in] nEndChar 要选择的结束位置
     * @return 返回选择的文字数量
     */
    int32_t SetSel(int32_t nStartChar, int32_t nEndChar);

    /** 替换所选内容
     * @param[in] lpszNewText 要替换的文字
     * @param[in] bCanUndo 是否可以撤销，true 为可以，否则为 false
     */
    void ReplaceSel(const DString& lpszNewText, bool bCanUndo);

    /** 获取所选文字内容
     * @return 返回所选文字内容
     */
    DString GetSelText() const;

    /** 获取指定范围的内容
     * @param[in] nStartChar 起始位置
     * @param[in] nEndChar 结束为止
     * @return 返回设置的指定位置的内容
     */
    DString GetTextRange(int32_t nStartChar, int32_t nEndChar) const;

    /** 设置隐藏或显示选择的文本
     * @param [in] bHideSelection 是否显示，true 为隐藏，false 为显示
     * @param [in] bChangeStyle 该参数未使用
     */
    void HideSelection(bool bHideSelection = true, bool bChangeStyle = false);

    /** 是否可以Redo
    */
    bool CanRedo() const;

    /** 重做操作
     * @return 成功返回 true，失败返回 false
     */
    bool Redo();

    /** 是否可撤销
    */
    bool CanUndo() const;

    /** 撤销操作
     * @return 成功返回 true，失败返回 false
     */
    bool Undo();

    /** 清空
     */
    void Clear();

    /** 复制所选内容
     */
    void Copy();

    /** 剪切所选内容
     */
    void Cut();

    /** 粘贴
     */
    void Paste();

    /** 检测是否可以粘贴
    */
    bool CanPaste() const;

    /** 获取总行数
     * @return 返回总行数
     */
    int32_t GetLineCount() const;

    /** 获取一行数据
     * @param[in] nIndex 要获取的行数
     * @param[in] nMaxLength 要获取当前行最大的数据长度
     * @return 返回获取的一行数据
     */
    DString GetLine(int32_t nIndex, int32_t nMaxLength) const;

    /** 获取指定行的第一个字符索引
     * @param[in] nLine 要获取第几行数据，默认为 -1
     * @return 返回指定行的第一个字符索引
     */
    int32_t LineIndex(int32_t nLine = -1) const;

    /** 获取指定行的数据长度
     * @param[in] nLine 要获取第几行数据，默认为 -1
     * @return 返回指定行的数据长度
     */
    int32_t LineLength(int32_t nLine = -1) const;

    /** 滚动文本
     * @param[in] nLines 指定垂直滚动方向
     * @return 成功返回 true，失败返回 false
     */
    bool LineScroll(int32_t nLines);

    /** 获取指定字符所在行数
     * @param[in] nIndex 字符的索引位置
     * @return 返回当前字符所在的行数
     */
    int32_t LineFromChar(int32_t nIndex) const;

    /** 获取指定位置字符的客户区坐标
     * @param[in] nChar 字符索引位置
     * @return 返回客户区坐标
     */
    UiPoint PosFromChar(int32_t nChar) const;

    /** 根据坐标返回指定字符索引
     * @param[in] pt 坐标信息
     * @return 返回最接近参数 pt 所指定的坐标位置
     */
    int32_t CharFromPos(UiPoint pt) const;

    /** 清空撤销列表
     */
    void EmptyUndoBuffer();

    /** 设置撤销列表容纳的内容数量
     * @param [in] nLimit
     * @return 返回设置后的撤销列表可容纳内容数量
     */
    uint32_t SetUndoLimit(uint32_t nLimit);

public:
    /** 向上一行
     */
    virtual void LineUp();

    /** 向下一行
     */
    virtual void LineDown();

    /** 向上翻页
     */
    virtual void PageUp() override;

    /** 向下翻页
     */
    virtual void PageDown() override;

    /** 返回到顶端
     */
    virtual void HomeUp() override;

    /** 返回到底部
     */
    virtual void EndDown();

    /** 水平向左滚动
     */
    virtual void LineLeft();

    /** 水平向右滚动
     */
    virtual void LineRight();

    /** 水平向左翻页
     */
    virtual void PageLeft() override;

    /** 水平向右翻页
     */
    virtual void PageRight() override;

    /** 返回到最左侧
     */
    virtual void HomeLeft() override;

    /** 返回到最后侧
     */
    virtual void EndRight() override;

protected:

    //一些基类的虚函数
    virtual bool CanPlaceCaptionBar() const override;
    virtual void OnInit() override;
    virtual uint32_t GetControlFlags() const override;

    //消息处理函数
    virtual bool OnSetCursor(const EventArgs& msg) override;
    virtual bool OnSetFocus(const EventArgs& msg) override;
    virtual bool OnKillFocus(const EventArgs& msg) override;
    virtual bool OnChar(const EventArgs& msg) override;
    virtual bool OnKeyDown(const EventArgs& msg) override;
    virtual bool OnImeStartComposition(const EventArgs& msg) override;
    virtual bool OnImeEndComposition(const EventArgs& msg) override;
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

private:
    /** 显示RichEdit上的菜单
    * @param [in] point 客户区的坐标
    */
    void ShowPopupMenu(const ui::UiPoint& point);

    /** 判断一个字符，是否在限制字符列表中
    */
    bool IsInLimitChars(wchar_t charValue) const;

    /** 判断是否可以进行粘贴操作(判断是否有字符限制、数字限制)
    * @return 如果返回true, 表示不可用进行粘贴操作
    */
    bool IsPasteLimited() const;

private:
    /** 触发文本变化事件
    */
    void OnTextChanged();

    /** 设置Spin功能的Class名称
    */
    bool SetSpinClass(const DString& spinClass);

    /** 获取文本内容，并转换为数字
    */
    int64_t GetTextNumber() const;

    /** 将数字转换为文本，设置文本内容
    */
    void SetTextNumber(int64_t nValue);

    /** 调整文本数字值
    */
    void AdjustTextNumber(int32_t nDelta);

    /** 开始启动调整文本数字值的定时器
    */
    void StartAutoAdjustTextNumberTimer(int32_t nDelta);

    /** 开始自动调整文本数字值
    */
    void StartAutoAdjustTextNumber(int32_t nDelta);

    /** 结束自动调整文本数字值
    */
    void StopAutoAdjustTextNumber();

private:
    /** 设置清除按钮功能的Class名称
    */
    void SetClearBtnClass(const DString& btnClass);

    /** 设置显示密码按钮功能的Class名称
    */
    void SetShowPasswordBtnClass(const DString& btnClass);

    /** 设置字体ID
    */
    void SetFontIdInternal(const DString& fontId);

    //文本横向和纵向对齐方式
    void SetHAlignType(HorAlignType alignType);
    void SetVAlignType(VerAlignType alignType);

    /** 绘制光标
     * @param[in] pRender 绘制引擎
     * @param[in] rcPaint 绘制位置
     */
    void PaintCaret(IRender* pRender, const UiRect& rcPaint);

    /** 切换光标是否显示
    */
    void ChangeCaretVisiable();

    /** 绘制提示文字
     * @param[in] pRender 绘制引擎
     */
    void PaintPromptText(IRender* pRender);

    /** 转发消息控件的实现
    */
    void OnMouseMessage(uint32_t uMsg, const EventArgs& msg);

    /** 获取粘贴板字符串
    */
    static void GetClipboardText(DStringW& out);

private:
    //一组供RichEditHost使用的函数
    friend class RichEditHost;

    /** 获取关联的窗口局部
    */
    HWND GetWindowHWND() const;

    /** 获取绘制的设备上下文
    */
    HDC GetDrawDC() const;

    /** 获取文本区域的矩形范围
    */
    UiSize GetNaturalSize(LONG width, LONG height);

    /** 设置输入法状态
    */
    void SetImmStatus(BOOL bOpen);

private:
    /** 调整光标的位置
    * @param [in] pt 需要设置调整的位置（客户区坐标），如果为(-1,-1)表示需要定位光标的位置
    */
    void AdjustCaretPos(const UiPoint& pt);

    /** 重绘
    */
    void Redraw();

private:
    bool m_bWantTab;            //是否接收TAB键，如果为true的时候，TAB键会当作文本输入，否则过滤掉TAB键
    bool m_bWantReturn;         //是否接收回车键，如果为true的时候，回车键会当作文本输入，否则过滤掉回车键
    bool m_bWantCtrlReturn;     //是否接收Ctrl + 回车键，如果为true的时候，回车键会当作文本输入，否则过滤掉回车键
     
    bool m_bSelAllEver;         //只在获取焦点后的第一次鼠标弹起全选

    bool m_bNoSelOnKillFocus;   //失去焦点的时候，取消文本选择（针对 m_bEnabled && IsReadOnly()）
    bool m_bSelAllOnFocus;      //获取焦点的时候，全选文本（针对 m_bEnabled && !IsReadOnly()）

    bool m_bIsComposition;      //输入法合成窗口是否可见

    bool m_bReadOnly;           //是否为只读模式
    bool m_bPasswordMode;       //是否为密码模式
    bool m_bShowPassword;       //是否显示密码
    wchar_t m_chPasswordChar;   //密码字符
    bool m_bFlashPasswordChar;  //是否短暂的显示密码字符，然后再隐藏

    bool m_bNumberOnly;         //是否只允许输入数字
    bool m_bWordWrap;           //当显示超出边界时，是否自动换行
    bool m_bMultiLine;          //是否支持多行文本

    int32_t m_nLimitText;       //最大文本字符数（仅当为正数的时候代表有限制）
    bool m_bModified;           //文本内容是否有修改

private:
    bool m_bNoCaretReadonly;    //只读模式下，不显示光标
    bool m_bIsCaretVisiable;    //光标是否可见
    int32_t m_iCaretPosX;       //光标X坐标
    int32_t m_iCaretPosY;       //光标Y坐标
    int32_t m_iCaretWidth;      //光标宽度
    int32_t m_iCaretHeight;     //光标高度
    UiString m_sCaretColor;     //光标颜色

    int32_t m_nLineHeight;      //行高，与字体有关

    WeakCallbackFlag m_drawCaretFlag;   //绘制光标的定时器生命周期

private:
    UiString m_sFontId;                 //字体ID
    UiString m_sTextColor;              //正常文本颜色
    UiString m_sDisabledTextColor;      //Disabled状态的文本颜色

    bool m_bAllowPrompt;                //是否支持提示文字
    UiString m_sPromptColor;            //提示文字颜色
    UiString m_sPromptText;             //提示文本内容（只有编辑框为空的时候显示）
    UiString m_sPromptTextId;           //提示文字ID

private:
    /** 获取焦点时，显示的图片
    */
    Image* m_pFocusedImage;

    /** 文本内边距
    */
    UiPadding16 m_rcTextPadding;

    /** 是否使用Control设置的光标
    */
    bool m_bUseControlCursor;

    /** 是否允许通过Ctrl + 滚轮来调整缩放比例
    */
    bool m_bEnableWheelZoom;

    /** 是否允许使用默认的右键菜单
    */
    bool m_bEnableDefaultContextMenu;

    /** 允许输入的字符列表
    */
    std::unique_ptr<wchar_t[]> m_pLimitChars;

    /** 是否禁止触发文本变化事件
    */
    bool m_bDisableTextChangeEvent;

    /** 设置允许的最大数字(仅当IsNumberOnly()为true的时候有效)
    */
    int32_t m_maxNumber;

    /** 设置允许的最小数字(仅当IsNumberOnly()为true的时候有效)
    */
    int32_t m_minNumber;

    /** Spin功能的容器
    */
    VBox* m_pSpinBox;

    /** 自动调整文本数字值的定时器生命周期管理
    */
    WeakCallbackFlag m_flagAdjustTextNumber;

    /** 清除功能的按钮(仅当非只读模式有效)
    */
    Control* m_pClearButton;

    /** 显示/隐藏密码按钮(仅当密码模式有效)
    */
    Control* m_pShowPasswordButton;

private:
    /** 文本内容
    */
    DStringW m_text;

    /** 文本水平对齐方式
    */
    HorAlignType m_hAlignType;

    /** 文本垂直对齐方式
    */
    VerAlignType m_vAlignType;

    /** 选择的起始字符
    */
    int32_t m_nSelStartIndex;

    /** 选择的结束字符
    */
    int32_t m_nSelEndCharIndex;

    /** 是否显示选择的文本(显示时：选择的文本背景色与正常文本不同)
    */
    bool m_bHideSelection;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_SDL

#endif // UI_CONTROL_RICHEDIT_SDL_H_
