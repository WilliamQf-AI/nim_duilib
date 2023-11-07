#include "ListCtrlDataView.h" 
#include "ListCtrl.h"
#include "duilib/Render/AutoClip.h"

//�����ࣺListCtrlDataView / ListCtrlDataLayout

namespace ui
{
ListCtrlDataView::ListCtrlDataView() :
    VirtualListBox(new ListCtrlDataLayout),
    m_pListCtrl(nullptr),
    m_nTopElementIndex(0)
{
    VirtualLayout* pVirtualLayout = dynamic_cast<VirtualLayout*>(GetLayout());
    SetVirtualLayout(pVirtualLayout);

    ListCtrlDataLayout* pDataLayout = dynamic_cast<ListCtrlDataLayout*>(GetLayout());
    ASSERT(pDataLayout != nullptr);
    if (pDataLayout != nullptr) {
        pDataLayout->SetDataView(this);
    }
}

ListCtrlDataView::~ListCtrlDataView() 
{
}

void ListCtrlDataView::SetListCtrl(ListCtrl* pListCtrl)
{
    m_pListCtrl = pListCtrl;
}

void ListCtrlDataView::Refresh()
{
    if ((m_pListCtrl != nullptr) && !m_pListCtrl->IsEnableRefresh()) {
        //ˢ�¹����Ѿ���ֹ
        return;
    }
    if (!HasDataProvider()) {
        return;
    }
    //����UI�ؼ��ĸ�������������ʾʹ��
    AjustItemCount();

    //ˢ��UI�ؼ�����ʾ����
    if (GetElementCount() > 0) {
        ReArrangeChild(true);
        Arrange();
    }
    OnRefresh();
}

void ListCtrlDataView::AjustItemCount()
{
    VirtualLayout* pVirtualLayout = GetVirtualLayout();
    if (pVirtualLayout == nullptr) {
        return;
    }

    //���������
    size_t nMaxItemCount = pVirtualLayout->AjustMaxItem(GetPosWithoutPadding());
    if (nMaxItemCount == 0) {
        return;
    }

    //��ǰ��������
    size_t nElementCount = GetElementCount();

    //��ǰ������
    size_t nItemCount = GetItemCount();

    //ˢ�º��������
    size_t nNewItemCount = nElementCount;
    if (nNewItemCount > nMaxItemCount) {
        nNewItemCount = nMaxItemCount;
    }

    if (nItemCount > nNewItemCount) {
        //��������������������¼�������������Ƴ��������������������Ӻ���ɾ����
        size_t n = nItemCount - nNewItemCount;
        for (size_t i = 0; i < n; ++i) {
            size_t itemCount = GetItemCount();
            if (itemCount > 1) {
                //���ٱ���һ��Header�ؼ�
                RemoveItemAt(itemCount - 1);
            }
        }
    }
    else if (nItemCount < nNewItemCount) {
        //���������������С���¼���������������������������ٵ�����
        size_t n = nNewItemCount - nItemCount;
        for (size_t i = 0; i < n; ++i) {
            Control* pControl = CreateElement();
            AddItem(pControl);
        }
    }
}

int32_t ListCtrlDataView::GetListCtrlWidth() const
{
    int32_t nToltalWidth = 0;
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return nToltalWidth;
    }
    ListCtrlHeader* pHeaderCtrl = m_pListCtrl->GetListCtrlHeader();
    if (pHeaderCtrl == nullptr) {
        return nToltalWidth;
    }
    size_t nColumnCount = pHeaderCtrl->GetColumnCount();
    for (size_t index = 0; index < nColumnCount; ++index) {
        ListCtrlHeaderItem* pHeaderItem = pHeaderCtrl->GetColumn(index);
        if ((pHeaderItem != nullptr) && pHeaderItem->IsColumnVisible()) {
            nToltalWidth += pHeaderItem->GetColumnWidth();
        }
    }
    return nToltalWidth;
}

void ListCtrlDataView::SetTopElementIndex(size_t nTopElementIndex)
{
    m_nTopElementIndex = nTopElementIndex;
}

size_t ListCtrlDataView::GetTopElementIndex() const
{
    return m_nTopElementIndex;
}

void ListCtrlDataView::SetDisplayDataItems(const std::vector<size_t>& itemIndexList)
{
    m_diplayItemIndexList = itemIndexList;
}

void ListCtrlDataView::GetDisplayDataItems(std::vector<size_t>& itemIndexList) const
{
    itemIndexList = m_diplayItemIndexList;
}

bool ListCtrlDataView::IsDataItemDisplay(size_t itemIndex) const
{
    auto iter = std::find(m_diplayItemIndexList.begin(), m_diplayItemIndexList.end(), itemIndex);
    return iter != m_diplayItemIndexList.end();
}

bool ListCtrlDataView::EnsureDataItemVisible(size_t itemIndex, bool bToTop)
{
    if (!Box::IsValidItemIndex(itemIndex) || (itemIndex >= GetElementCount())) {
        return false;
    }
    VirtualLayout* pVirtualLayout = dynamic_cast<VirtualLayout*>(GetLayout());
    if (pVirtualLayout != nullptr) {
        pVirtualLayout->EnsureVisible(GetRect(), itemIndex, bToTop);
        return true;
    }
    return false;
}

size_t ListCtrlDataView::GetTopDataItemIndex(int64_t nScrollPosY) const
{
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return 0;
    }
    size_t itemIndex = 0;
    if (nScrollPosY <= 0) {
        return itemIndex;
    }
    ListCtrlDataProvider* pDataProvider = dynamic_cast<ListCtrlDataProvider*>(GetDataProvider());
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return itemIndex;
    }
    const ListCtrlDataProvider::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = itemDataList.size();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlRowData& rowData = itemDataList[index];
        nItemHeight = rowData.nItemHeight;
        if (nItemHeight < 0) {
            //ȡĬ���и�
            nItemHeight = m_pListCtrl->GetDataItemHeight();
        }
        if (!rowData.bVisible || (nItemHeight  == 0)){
            continue;
        }
        totalItemHeight += nItemHeight;
        //���ÿ�и߶ȶ���ͬ���൱�� nScrollPosY / ItemHeight
        if (totalItemHeight > nScrollPosY) {
            itemIndex = index;
            break;
        }
    }
    return itemIndex;
}

int32_t ListCtrlDataView::GetDataItemHeight(size_t itemIndex) const
{
    int32_t nItemHeight = 0;
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return 0;
    }
    nItemHeight = m_pListCtrl->GetDataItemHeight();
    ListCtrlDataProvider* pDataProvider = dynamic_cast<ListCtrlDataProvider*>(GetDataProvider());
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return 0;
    }
    const ListCtrlDataProvider::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    ASSERT(itemIndex < itemDataList.size());
    if (itemIndex < itemDataList.size()) {
        if (itemDataList[itemIndex].nItemHeight >= 0) {
            nItemHeight = itemDataList[itemIndex].nItemHeight;
        }        
    }
    return nItemHeight;
}

void ListCtrlDataView::GetDataItemsToShow(int64_t nScrollPosY, size_t maxCount, 
                                          std::vector<ShowItemInfo>& itemIndexList,
                                          std::vector<ShowItemInfo>& atTopItemIndexList,
                                          int64_t& nPrevItemHeights) const
{
    nPrevItemHeights = 0;
    itemIndexList.clear();
    atTopItemIndexList.clear();
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return;
    }
    ASSERT(nScrollPosY >= 0);
    if (nScrollPosY < 0) {
        return;
    }
    ASSERT(maxCount != 0);
    if (maxCount == 0) {
        return;
    }
    ListCtrlDataProvider* pDataProvider = dynamic_cast<ListCtrlDataProvider*>(GetDataProvider());
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return;
    }
    size_t nTopDataItemIndex = Box::InvalidIndex; //�����ɼ��ĵ�һ��Ԫ�����
    const int32_t nDefaultItemHeight = m_pListCtrl->GetDataItemHeight(); //Ĭ���и�
    //�ö���Ԫ�����
    struct AlwaysAtTopData
    {
        int8_t nAlwaysAtTop;    //�ö����ȼ�
        size_t nItemIndex;      //Ԫ������
        int32_t nItemHeight;    //Ԫ�صĸ߶�
    };
    std::vector<AlwaysAtTopData> alwaysAtTopItemList;
    
    const ListCtrlDataProvider::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = itemDataList.size();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlRowData& rowData = itemDataList[index];
        nItemHeight = (rowData.nItemHeight < 0) ? nDefaultItemHeight : rowData.nItemHeight;
        if (!rowData.bVisible || (nItemHeight == 0)) {
            //���ɼ��ģ�����
            continue;
        }

        if (rowData.nAlwaysAtTop >= 0) {
            //�ö���Ԫ��
            if (alwaysAtTopItemList.size() < maxCount) {
                alwaysAtTopItemList.push_back({ rowData.nAlwaysAtTop, index, nItemHeight });
            }
            continue;
        }

        if (nTopDataItemIndex == Box::InvalidIndex) {
            totalItemHeight += nItemHeight;
            //���ÿ�и߶ȶ���ͬ���൱�� nScrollPosY / ItemHeight
            if (totalItemHeight > nScrollPosY) {
                nTopDataItemIndex = index;
                nPrevItemHeights = totalItemHeight - nItemHeight;
            }
        }

        if ((nTopDataItemIndex != Box::InvalidIndex) && (itemIndexList.size() < maxCount)){
            itemIndexList.push_back({ index, nItemHeight });
        }
    }

    //���ö�������
    if (!alwaysAtTopItemList.empty()) {
        std::stable_sort(alwaysAtTopItemList.begin(), alwaysAtTopItemList.end(),
            [](const AlwaysAtTopData& a, const AlwaysAtTopData& b) {
                //nAlwaysAtTopֵ��ģ�����ǰ��
                return a.nAlwaysAtTop > b.nAlwaysAtTop;
            });
    }
    for (const AlwaysAtTopData& item : alwaysAtTopItemList) {
        atTopItemIndexList.push_back({ item.nItemIndex, item.nItemHeight });
    }
    if (atTopItemIndexList.size() >= maxCount) {
        atTopItemIndexList.resize(maxCount);
        itemIndexList.clear();
    }
    else {
        size_t nLeftCount = maxCount - atTopItemIndexList.size();
        if (itemIndexList.size() > nLeftCount) {
            itemIndexList.resize(nLeftCount);
        }
    }
    ASSERT((itemIndexList.size() + atTopItemIndexList.size()) <= maxCount);
}

int32_t ListCtrlDataView::GetMaxDataItemsToShow(int64_t nScrollPosY, int32_t nRectHeight, 
                          std::vector<size_t>* pItemIndexList,
                          std::vector<size_t>* pAtTopItemIndexList) const
{
    if (pItemIndexList) {
        pItemIndexList->clear();
    }
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return 0;
    }
    ASSERT(nScrollPosY >= 0);
    if (nScrollPosY < 0) {
        return 0;
    }
    ASSERT(nRectHeight > 0);
    if (nRectHeight <= 0) {
        return 0;
    }
    ListCtrlDataProvider* pDataProvider = dynamic_cast<ListCtrlDataProvider*>(GetDataProvider());
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return 0;
    }
    size_t nTopDataItemIndex = Box::InvalidIndex; //�����ɼ��ĵ�һ��Ԫ�����
    const int32_t nDefaultItemHeight = m_pListCtrl->GetDataItemHeight(); //Ĭ���и�
    //�ö���Ԫ�����
    struct AlwaysAtTopData
    {
        int8_t nAlwaysAtTop;
        size_t index;
    };
    std::vector<AlwaysAtTopData> alwaysAtTopItemList;
    std::vector<size_t> itemIndexList;

    const ListCtrlDataProvider::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = itemDataList.size();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlRowData& rowData = itemDataList[index];
        nItemHeight = (rowData.nItemHeight < 0) ? nDefaultItemHeight : rowData.nItemHeight;
        if (!rowData.bVisible || (nItemHeight == 0)) {
            //���ɼ��ģ�����
            continue;
        }

        if (rowData.nAlwaysAtTop >= 0) {
            //�ö���Ԫ��
            alwaysAtTopItemList.push_back({ rowData.nAlwaysAtTop, index });
            continue;
        }

        if (nTopDataItemIndex == Box::InvalidIndex) {
            totalItemHeight += nItemHeight;
            //���ÿ�и߶ȶ���ͬ���൱�� nScrollPosY / ItemHeight
            if (totalItemHeight > nScrollPosY) {
                nTopDataItemIndex = index;
            }
        }

        if (nTopDataItemIndex != Box::InvalidIndex) {
            itemIndexList.push_back(index);
        }
    }

    //���ö�������
    if (!alwaysAtTopItemList.empty()) {
        std::stable_sort(alwaysAtTopItemList.begin(), alwaysAtTopItemList.end(),
            [](const AlwaysAtTopData& a, const AlwaysAtTopData& b) {
                //nAlwaysAtTopֵ��ģ�����ǰ��
                return a.nAlwaysAtTop > b.nAlwaysAtTop;
            });
    }
    std::vector<size_t> atTopIndexList;
    std::vector<size_t> tempItemIndexList;
    tempItemIndexList.swap(itemIndexList);
    for (const AlwaysAtTopData& item : alwaysAtTopItemList) {
        itemIndexList.push_back(item.index);
        atTopIndexList.push_back(item.index);
    }
    for (size_t index : tempItemIndexList) {
        itemIndexList.push_back(index);
    }
    int32_t nShowItemCount = 0;
    int64_t nTotalHeight = 0;
    for (size_t index : itemIndexList) {
        if (index >= dataItemCount) {
            continue;
        }
        const ListCtrlRowData& rowData = itemDataList[index];
        nItemHeight = rowData.nItemHeight;
        if (nItemHeight < 0) {
            //ȡĬ���и�
            nItemHeight = nDefaultItemHeight;
        }
        nTotalHeight += nItemHeight;
        if (nTotalHeight < nRectHeight) {
            if (pItemIndexList) {
                pItemIndexList->push_back(index);
            }
            if (pAtTopItemIndexList != nullptr) {
                if (std::find(atTopIndexList.begin(),
                              atTopIndexList.end(), index) != atTopIndexList.end()) {
                    pAtTopItemIndexList->push_back(index);
                }
            }
            ++nShowItemCount;
        }
        else {
            nShowItemCount += 2;
            break;
        }
    }
    return nShowItemCount;
}

int64_t ListCtrlDataView::GetDataItemTotalHeights(size_t itemIndex, bool bIncludeAtTops) const
{
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return 0;
    }
    ListCtrlDataProvider* pDataProvider = dynamic_cast<ListCtrlDataProvider*>(GetDataProvider());
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return 0;
    }
    const int32_t nDefaultItemHeight = m_pListCtrl->GetDataItemHeight(); //Ĭ���и�
    const ListCtrlDataProvider::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = itemDataList.size();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlRowData& rowData = itemDataList[index];
        nItemHeight = (rowData.nItemHeight < 0) ? nDefaultItemHeight : rowData.nItemHeight;
        if (!rowData.bVisible || (nItemHeight == 0)) {
            //���ɼ��ģ�����
            continue;
        }

        if (rowData.nAlwaysAtTop >= 0) {
            //�ö���Ԫ�أ���Ҫͳ������
            if (bIncludeAtTops) {
                totalItemHeight += nItemHeight;
            }            
        }
        else if (index < itemIndex) {
            //����Ҫ���Ԫ��
            totalItemHeight += nItemHeight;
        }
        else if (!bIncludeAtTops) {
            //�Ѿ����
            break;
        }
    }
    return totalItemHeight;
}

bool ListCtrlDataView::IsNormalMode() const
{
    bool bNormalMode = true;
    ListCtrlDataProvider* pDataProvider = dynamic_cast<ListCtrlDataProvider*>(GetDataProvider());
    if (pDataProvider != nullptr) {
        bNormalMode = pDataProvider->IsNormalMode();
    }
    return bNormalMode;
}

void ListCtrlDataView::SetAtTopControlIndex(const std::vector<size_t>& atTopControlList)
{
    m_atTopControlList = atTopControlList;
}

void ListCtrlDataView::MoveTopItemsToLast(std::vector<Control*>& items, std::vector<Control*>& atTopItems) const
{
    atTopItems.clear();
    if (items.empty()) {
        return;
    }
    for (size_t index : m_atTopControlList) {
        if (index < items.size()) {
            atTopItems.push_back(items[index]);
        }
    }
    for (int32_t i = (int32_t)m_atTopControlList.size() - 1; i >= 0; --i) {
        size_t index = m_atTopControlList[i];
        if (index < items.size()) {
            items.erase(items.begin() + index);
        }
    }
    for (Control* pControl : atTopItems) {
        items.push_back(pControl);
    }
    ListCtrlHeader* pHeaderCtrl = dynamic_cast<ListCtrlHeader*>(GetItemAt(0));
    if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
        //������Header�ؼ������ⱻ�����ĸ���
        Control* pHeader = items.front();
        items.erase(items.begin());
        items.push_back(pHeader);

        //��Header�ؼ����ӵ��б�
        atTopItems.push_back(pHeader);
    }    
    ASSERT(items.size() == m_items.size());
}

void ListCtrlDataView::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    //��дVirtualListBox::PaintChild / ScrollBox::PaintChild������ȷ��Header��������
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
        return;
    }

    const size_t nItemCount = GetItemCount();
    if (nItemCount <= 1) {
        //�����Ǳ�ͷ��ֱ�ӻ���
        __super::PaintChild(pRender, rcPaint);
        return;
    }
    ListCtrlHeader* pHeaderCtrl = dynamic_cast<ListCtrlHeader*>(GetItemAt(0));
    if ((pHeaderCtrl == nullptr) || !pHeaderCtrl->IsVisible()) {
        __super::PaintChild(pRender, rcPaint);
        return;
    }

    //��Ҫ�������Ʋ���
    ReArrangeChild(false);
    std::vector<Control*> items = m_items;
    //Header���ö���Ԫ�أ��ƶ������
    std::vector<Control*> atTopItems;
    MoveTopItemsToLast(items, atTopItems);

    UiRect rcTopControls; //�����ö��ؼ��ľ�������
    for (size_t i = 0; i < atTopItems.size(); ++i) {
        const Control* pTopControl = atTopItems[i];
        if (i == 0) {
            rcTopControls = pTopControl->GetRect();
        }
        else {
            rcTopControls.Union(pTopControl->GetRect());
        }
    }
    for (Control* pControl : items) {
        if (pControl == nullptr) {
            continue;
        }
        if (!pControl->IsVisible()) {
            continue;
        }

        UiSize scrollPos = GetScrollOffset();
        UiRect rcNewPaint = GetPosWithoutPadding();
        AutoClip alphaClip(pRender, rcNewPaint, IsClip());
        rcNewPaint.Offset(scrollPos.cx, scrollPos.cy);
        rcNewPaint.Offset(GetRenderOffset().x, GetRenderOffset().y);

        bool bHasClip = false;
        if (!atTopItems.empty() &&
            (std::find(atTopItems.begin(), atTopItems.end(), pControl) == atTopItems.end())) {            
            UiRect rcControlRect = pControl->GetRect();
            UiRect rUnion;
            if (UiRect::Intersect(rUnion, rcTopControls, rcControlRect)) {
                //�н�������Ҫ���òü�����������ö�Ԫ��������Ԫ���ص�������
                pRender->SetClip(rUnion, false);
                bHasClip = true;
            }
        }

        UiPoint ptOffset(scrollPos.cx, scrollPos.cy);
        UiPoint ptOldOrg = pRender->OffsetWindowOrg(ptOffset);
        pControl->AlphaPaint(pRender, rcNewPaint);
        pRender->SetWindowOrg(ptOldOrg);
        if (bHasClip) {
            pRender->ClearClip();
        }
    }
    ScrollBar* pVScrollBar = GetVScrollBar();
    ScrollBar* pHScrollBar = GetHScrollBar();
    if ((pHScrollBar != nullptr) && pHScrollBar->IsVisible()) {
        pHScrollBar->AlphaPaint(pRender, rcPaint);
    }

    if ((pVScrollBar != nullptr) && pVScrollBar->IsVisible()) {
        pVScrollBar->AlphaPaint(pRender, rcPaint);
    }
}

Control* ListCtrlDataView::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags, UiPoint scrollPos)
{
    //���£�Box::FindControl ��������Header���ȱ����ҵ���ֻ��������UIFIND_TOP_FIRST��־�����
    Control* pFoundControl = __super::FindControl(Proc, pData, uFlags, scrollPos);
    if ((uFlags & UIFIND_TOP_FIRST) == 0) {
        return pFoundControl;
    }
    else {
        if (dynamic_cast<ScrollBar*>(pFoundControl) != nullptr) {
            //����ڹ��������棬ֱ�ӷ���
            return pFoundControl;
        }
    }
   
    // Check if this guy is valid
    if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) {
        return nullptr;
    }
    if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) {
        return nullptr;
    }
    if ((uFlags & UIFIND_HITTEST) != 0) {
        ASSERT(pData != nullptr);
        UiPoint pt(*(static_cast<UiPoint*>(pData)));
        if ((pData != nullptr) && !GetRect().ContainsPt(pt)) {
            return nullptr;
        }
        if (!IsMouseChildEnabled()) {
            Control* pResult = Control::FindControl(Proc, pData, uFlags);
            return pResult;
        }
    }

    if ((uFlags & UIFIND_ME_FIRST) != 0) {
        Control* pControl = Control::FindControl(Proc, pData, uFlags);
        if (pControl != nullptr) {
            return pControl;
        }
    }
    const std::vector<Control*>& items = m_items;
    UiRect rc = GetRectWithoutPadding();
    if ((uFlags & UIFIND_TOP_FIRST) != 0) {
        std::vector<Control*> newItems = m_items;
        //Header���ö���Ԫ�أ��ƶ������
        std::vector<Control*> atTopItems;
        MoveTopItemsToLast(newItems, atTopItems);
        for (int it = (int)newItems.size() - 1; it >= 0; --it) {
            if (newItems[it] == nullptr) {
                continue;
            }
            Control* pControl = nullptr;
            if ((uFlags & UIFIND_HITTEST) != 0) {
                ASSERT(pData != nullptr);
                if (pData != nullptr) {
                    UiPoint newPoint(*(static_cast<UiPoint*>(pData)));
                    newPoint.Offset(scrollPos);
                    pControl = newItems[it]->FindControl(Proc, &newPoint, uFlags);
                }
            }
            else {
                pControl = newItems[it]->FindControl(Proc, pData, uFlags);
            }
            if (pControl != nullptr) {
                if ((uFlags & UIFIND_HITTEST) != 0 &&
                    !pControl->IsFloat() &&
                    (pData != nullptr) &&
                    !rc.ContainsPt(*(static_cast<UiPoint*>(pData)))) {
                    continue;
                }
                else {
                    return pControl;
                }
            }
        }
    }
    else {
        for (Control* pItemControl : items) {
            if (pItemControl == nullptr) {
                continue;
            }
            Control* pControl = nullptr;
            if ((uFlags & UIFIND_HITTEST) != 0) {
                ASSERT(pData != nullptr);
                if (pData != nullptr) {
                    UiPoint newPoint(*(static_cast<UiPoint*>(pData)));
                    newPoint.Offset(scrollPos);
                    pControl = pItemControl->FindControl(Proc, &newPoint, uFlags);
                }
            }
            else {
                pControl = pItemControl->FindControl(Proc, pData, uFlags);
            }
            if (pControl != nullptr) {
                if ((uFlags & UIFIND_HITTEST) != 0 &&
                    !pControl->IsFloat() &&
                    (pData != nullptr) &&
                    !rc.ContainsPt(*(static_cast<UiPoint*>(pData)))) {
                    continue;
                }
                else {
                    return pControl;
                }
            }
        }
    }

    Control* pResult = nullptr;
    if ((uFlags & UIFIND_ME_FIRST) == 0) {
        pResult = Control::FindControl(Proc, pData, uFlags);
    }
    return pResult;
}

size_t ListCtrlDataView::GetDisplayItemCount(bool /*bIsHorizontal*/, size_t& nColumns, size_t& nRows) const
{
    nColumns = 1;
    size_t nDiplayItemCount = m_diplayItemIndexList.size();
    size_t nAtTopItemCount = m_atTopControlList.size();
    nRows = nDiplayItemCount;
    if (nRows > nAtTopItemCount) {
        nRows -= nAtTopItemCount;//��ȥ�ö���
    }
    if (nRows > 1) {
        if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetHeaderHeight() > 0)) {
            nRows -= 1;//��ȥHeader
        }
    }
    return nRows * nColumns;
}

bool ListCtrlDataView::IsSelectableElement(size_t nElementIndex) const
{
    bool bSelectable = true;
    ListCtrlDataProvider* pDataProvider = dynamic_cast<ListCtrlDataProvider*>(GetDataProvider());
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider != nullptr) {
        const ListCtrlDataProvider::RowDataList& itemDataList = pDataProvider->GetItemDataList();
        if (nElementIndex < itemDataList.size()) {
            const ListCtrlRowData& rowData = itemDataList[nElementIndex];
            bSelectable = rowData.bVisible && (rowData.nAlwaysAtTop < 0);
        }
    }
    return bSelectable;
}

size_t ListCtrlDataView::FindSelectableElement(size_t nElementIndex, bool bForward) const
{
    ListCtrlDataProvider* pDataProvider = dynamic_cast<ListCtrlDataProvider*>(GetDataProvider());
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return nElementIndex;
    }
    const ListCtrlDataProvider::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    const size_t nElementCount = itemDataList.size();
    if ((nElementCount == 0) || (nElementIndex >= nElementCount)) {
        return nElementIndex;
    }
    bool bSelectable = itemDataList[nElementIndex].nAlwaysAtTop < 0;
    if (!itemDataList[nElementIndex].bVisible) {
        bSelectable = false;
    }
    if (!bSelectable) {
        size_t nStartIndex = nElementIndex;
        nElementIndex = Box::InvalidIndex;
        if (bForward) {
            //��ǰ������һ�������ö���
            for (size_t i = nStartIndex + 1; i < nElementCount; ++i) {
                const ListCtrlRowData& rowData = itemDataList[i];
                if (rowData.bVisible && (rowData.nAlwaysAtTop < 0)) {
                    nElementIndex = i;
                    break;
                }
            }
        }
        else {
            //��������һ�������ö���
            for (int32_t i = (int32_t)nStartIndex - 1; i >= 0; --i) {
                const ListCtrlRowData& rowData = itemDataList[i];
                if (rowData.bVisible && (rowData.nAlwaysAtTop < 0)) {
                    nElementIndex = i;
                    break;
                }
            }
        }        
    }
    return nElementIndex;
}

void ListCtrlDataView::OnRefresh()
{
}

void ListCtrlDataView::OnArrangeChild()
{
    if (m_pListCtrl != nullptr) {
        m_pListCtrl->UpdateControlCheckStatus(Box::InvalidIndex);
    }
}

ListCtrlDataLayout::ListCtrlDataLayout():
    m_pDataView(nullptr),
    m_bReserveSet(false)
{
    m_nReserveHeight = GlobalManager::Instance().Dpi().GetScaleInt(8);
}

void ListCtrlDataLayout::SetDataView(ListCtrlDataView* pDataView)
{
    ASSERT(pDataView != nullptr);
    m_pDataView = pDataView;
}

UiSize64 ListCtrlDataLayout::ArrangeChild(const std::vector<ui::Control*>& /*items*/, ui::UiRect rc)
{
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return UiSize64();
    }
    DeflatePadding(rc);
    const int32_t nHeaderHeight = GetHeaderHeight();
    int64_t nTotalHeight = GetElementsHeight(Box::InvalidIndex, true) + nHeaderHeight;
    UiSize64 sz(rc.Width(), rc.Height());
    sz.cy = std::max(nTotalHeight, sz.cy);
    m_bReserveSet = false;
    if (nTotalHeight > rc.Height()) {
        //��Ҫ���ֹ��������ײ�Ԥ���ռ�
        sz.cy += m_nReserveHeight;
        if (pDataView->GetHScrollBar() != nullptr) {
            sz.cy += pDataView->GetHScrollBar()->GetHeight();
        }
        m_bReserveSet = true;
    }
    sz.cx = std::max(GetItemWidth(), rc.Width()); //�������ֺ��������
    LazyArrangeChild(rc);
    return sz;
}

UiSize ListCtrlDataLayout::EstimateSizeByChild(const std::vector<Control*>& /*items*/, ui::UiSize szAvailable)
{
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return UiSize();
    }
    szAvailable.Validate();
    UiEstSize estSize;
    if (GetOwner() != nullptr) {
        estSize = GetOwner()->Control::EstimateSize(szAvailable);
    }
    UiSize size(estSize.cx.GetInt32(), estSize.cy.GetInt32());
    if (estSize.cx.IsStretch()) {
        size.cx = CalcStretchValue(estSize.cx, szAvailable.cx);
    }
    if (estSize.cy.IsStretch()) {
        size.cy = CalcStretchValue(estSize.cy, szAvailable.cy);
    }
    if (size.cx == 0) {
        size.cx = GetItemWidth();
    }
    if (size.cy == 0) {
        size.cy = szAvailable.cy;
    }
    size.Validate();
    return size;
}

void ListCtrlDataLayout::LazyArrangeChild(UiRect rc) const
{
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return;
    }
    UiRect orgRect = rc;

    //��ջ�������
    pDataView->SetAtTopControlIndex(std::vector<size_t>());
    pDataView->SetTopElementIndex(Box::InvalidIndex);
    pDataView->SetDisplayDataItems(std::vector<size_t>());

    if (pDataView->IsNormalMode()) {
        //����ģʽ
        LazyArrangeChildNormal(rc);
        return;
    }

    const size_t nItemCount = pDataView->GetItemCount();
    ASSERT(nItemCount > 0);
    if (nItemCount <= 1) {
        //��һ��Ԫ���Ǳ�ͷ        
        pDataView->OnArrangeChild();
        return;
    }

    //��һ��Ԫ���Ǳ�ͷ�ؼ���������λ�ô�С
    Control* pHeaderCtrl = pDataView->GetItemAt(0);
    if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
        int32_t nHeaderHeight = pHeaderCtrl->GetFixedHeight().GetInt32();
        if (nHeaderHeight > 0) {
            //��ͷ�Ŀ���
            int32_t nHeaderWidth = std::max(GetItemWidth(), rc.Width());
            if (nHeaderWidth <= 0) {
                nHeaderWidth = rc.Width();
            }
            ui::UiRect rcTile(rc.left, rc.top, rc.left + nHeaderWidth, rc.top + nHeaderHeight);
            pHeaderCtrl->SetPos(rcTile);
            rc.top += nHeaderHeight;
        }
    }

    //��¼�ɼ���Ԫ���������б�
    std::vector<size_t> diplayItemIndexList;

    //��������Y����λ��
    int64_t nScrollPosY = pDataView->GetScrollPos().cy;

    //���㵱ǰ�����ܹ���ʾ����������
    int32_t nCalcItemCount = pDataView->GetMaxDataItemsToShow(nScrollPosY, rc.Height());
    if (nCalcItemCount > (nItemCount - 1)) {
        //UI�ؼ��ĸ������㣬���µ���
        pDataView->AjustItemCount();
    }

    //ȡ����Ҫ��ʾ������Ԫ������б�
    std::vector<ListCtrlDataView::ShowItemInfo> showItemIndexList;
    std::vector<ListCtrlDataView::ShowItemInfo> atTopItemIndexList;
    int64_t nPrevItemHeights = 0;
    pDataView->GetDataItemsToShow(nScrollPosY, nItemCount - 1, 
                                  showItemIndexList, atTopItemIndexList, nPrevItemHeights);
    if (showItemIndexList.empty() && atTopItemIndexList.empty()) {
        //û����Ҫ��ʾ������
        pDataView->OnArrangeChild();
        return;
    }

    // ����Ԫ�ص�������
    size_t nTopElementIndex = Box::InvalidIndex;
    if (!atTopItemIndexList.empty()) {
        nTopElementIndex = atTopItemIndexList.front().nItemIndex;
    }
    else if (!showItemIndexList.empty()) {
        nTopElementIndex = showItemIndexList.front().nItemIndex;
    }
    pDataView->SetTopElementIndex(nTopElementIndex); 

    //��������ƫ�ƣ������������ϴ�ʱ��rc���32λ�ľ��εĸ߶Ȼ�Խ�磬��Ҫ64λ���Ͳ�������
    pDataView->SetScrollVirtualOffsetY(nScrollPosY);

    //��һ������Y�������ƫ�ƣ���Ҫ���֣��������λ�ñ䶯������ˢ�½������ƫ��������һ��������ʾ������
    int32_t yOffset = 0;
    if ((nScrollPosY > 0) && !showItemIndexList.empty()) {
        int32_t nFirstHeight = showItemIndexList.front().nItemHeight;
        if (nFirstHeight > 0) {            
            yOffset = std::abs(nScrollPosY - nPrevItemHeights) % nFirstHeight;
        }
    }
    if ((nScrollPosY > 0) && (nScrollPosY == pDataView->GetScrollRange().cy)) {
        //�������Ѿ�����
        if (!showItemIndexList.empty()) {
            size_t nLastItemIndex = showItemIndexList[showItemIndexList.size() - 1].nItemIndex;
            if (nLastItemIndex == pDataView->GetElementCount() - 1) {
                //�Ѿ������һ����¼��ȷ���ײ���ʾ����
                int32_t rcHeights = rc.Height();
                if (m_bReserveSet && (rcHeights > m_nReserveHeight)) {
                    rcHeights -= m_nReserveHeight;
                    if (pDataView->GetHScrollBar() != nullptr) {
                        rcHeights -= pDataView->GetHScrollBar()->GetHeight();
                    }                    
                }
                for (const auto& info : atTopItemIndexList) {
                    rcHeights -= info.nItemHeight;
                }
                for (int32_t nIndex = (int32_t)showItemIndexList.size() - 1; nIndex >= 0; --nIndex) {
                    const auto& info = showItemIndexList[nIndex];
                    if ((rcHeights - info.nItemHeight) > 0) {
                        rcHeights -= info.nItemHeight;
                    }
                }
                if (rcHeights > 0) {
                    int32_t nFirstHeight = 0;
                    if (!showItemIndexList.empty()) {
                        nFirstHeight = showItemIndexList.front().nItemHeight;
                    }
                    if (nFirstHeight > 0) {
                        yOffset = nFirstHeight - (rcHeights % nFirstHeight);
                    }
                }
            }
        }
    }

    struct ShowItemIndex
    {
        size_t nElementIndex;   //Ԫ������
        int32_t nItemHeight;    //Ԫ�صĸ߶�
        bool bAtTop;            //�Ƿ��ö�
        int32_t yOffset;        //Y��ƫ����
    };
    std::vector<ShowItemIndex> itemIndexList;
    for (const auto& info : atTopItemIndexList) {
        itemIndexList.push_back({ info.nItemIndex, info.nItemHeight, true, 0});
    }
    for (const auto& info : showItemIndexList) {
        itemIndexList.push_back({ info.nItemIndex, info.nItemHeight, false, yOffset });
        yOffset = 0;    //ֻ�е�һ��Ԫ������ƫ��
    }
    //Ԫ�صĿ��ȣ�����Ԫ�ؿ��ȶ���ͬ
    const int32_t cx = std::max(GetItemWidth(), rc.Width());
    ASSERT(cx > 0);

    //�ؼ������Ͻ�����ֵ
    ui::UiPoint ptTile(rc.left, rc.top);

    UiSize szItem;
    size_t iCount = 0;
    std::vector<size_t> atTopUiItemIndexList;
    //��һ��Ԫ���Ǳ�ͷ�ؼ�������������ݣ����Դ�1��ʼ
    for (size_t index = 1; index < nItemCount; ++index) {
        Control* pControl = pDataView->GetItemAt(index);
        if (pControl == nullptr) {
            continue;
        }

        bool bAlwaysAtTop = false;
        size_t nElementIndex = Box::InvalidIndex;
        bool bFillElement = true;
        if (iCount < itemIndexList.size()) {
            //��ǰ����Ԫ�ص�������
            const ShowItemIndex& showItemIndex = itemIndexList[iCount];
            nElementIndex = showItemIndex.nElementIndex;
            bAlwaysAtTop = showItemIndex.bAtTop;
            szItem.cx = cx;
            szItem.cy = showItemIndex.nItemHeight;
            ASSERT(szItem.cy > 0);

            //���õ�ǰ�ؼ��Ĵ�С��λ��
            if (showItemIndex.yOffset > 0) {
                ptTile.y = ptTile.y - showItemIndex.yOffset;
            }
            UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + szItem.cx, ptTile.y + szItem.cy);
            pControl->SetPos(rcTile);
            if (rcTile.top > orgRect.bottom) {
                //���Ԫ���Ѿ����ɼ�����ֹͣ�������
                bFillElement = false;                
            }
        }
        else {
            //�����Ѿ�չʾ���
            bFillElement = false;
        }

        if (nElementIndex >= pDataView->GetElementCount()) {
            bFillElement = false;
        }
        
        if (bFillElement) {
            if (!pControl->IsVisible()) {
                pControl->SetVisible(true);
            }
            // �������
            //TODO: �Ż����룬����ÿ��ˢ�¶�Fill
            pDataView->FillElement(pControl, nElementIndex);
            diplayItemIndexList.push_back(nElementIndex);

            ListCtrlItem* pListCtrlItem = dynamic_cast<ListCtrlItem*>(pControl);
            if (pListCtrlItem != nullptr) {
                //�ö������ѡ��
                pListCtrlItem->SetSelectableType(bAlwaysAtTop ? false : true);
            }
            if (bAlwaysAtTop) {
                //��¼�ö���
                atTopUiItemIndexList.push_back(index);
            }
        }
        else {
            if (pControl->IsVisible()) {
                pControl->SetVisible(false);
            }
        }

        //�л�����һ��
        ptTile.y += szItem.cy + GetChildMarginY();
        ++iCount;
    }

    pDataView->SetAtTopControlIndex(atTopUiItemIndexList);
    pDataView->SetDisplayDataItems(diplayItemIndexList);
    pDataView->OnArrangeChild();
}

void ListCtrlDataLayout::LazyArrangeChildNormal(UiRect rc) const
{    
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return;
    }

    UiRect orgRect = rc;
    const size_t nItemCount = pDataView->GetItemCount();
    if (nItemCount > 0) {
        //��һ��Ԫ���Ǳ�ͷ�ؼ���������λ�ô�С
        Control* pHeaderCtrl = pDataView->GetItemAt(0);
        if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
            int32_t nHeaderHeight = pHeaderCtrl->GetFixedHeight().GetInt32();
            if (nHeaderHeight > 0) {
                int32_t nHeaderWidth = GetElementSize(rc.Width(), 0).cx;
                if (nHeaderWidth <= 0) {
                    nHeaderWidth = rc.Width();
                }
                ui::UiRect rcTile(rc.left, rc.top, rc.left + nHeaderWidth, rc.top + nHeaderHeight);
                pHeaderCtrl->SetPos(rcTile);
                rc.top += nHeaderHeight;
            }
        }
    }

    //Ԫ�صĿ��ȣ�����Ԫ�ؿ��ȶ���ͬ
    const int32_t cx = std::max(GetItemWidth(), rc.Width());
    ASSERT(cx > 0);

    //Ԫ�صĸ߶ȣ�����Ԫ�ظ߶ȶ���ͬ
    const int32_t cy = GetItemHeight();
    ASSERT(cy > 0);

    //��¼�ɼ���Ԫ���������б�
    std::vector<size_t> diplayItemIndexList;

    // ����Ԫ�ص�������
    const size_t nTopElementIndex = GetTopElementIndex(orgRect);
    pDataView->SetTopElementIndex(nTopElementIndex);

    //��������Y����λ��
    int64_t nScrollPosY = pDataView->GetScrollPos().cy;

    //Y�������ƫ�ƣ���Ҫ���֣��������λ�ñ䶯������ˢ�½������ƫ��
    int32_t yOffset = 0;
    if (cy > 0) {
        yOffset = TruncateToInt32(nScrollPosY % cy);
    }

    //��������ƫ�ƣ������������ϴ�ʱ��rc���32λ�ľ��εĸ߶Ȼ�Խ�磬��Ҫ64λ���Ͳ�������
    pDataView->SetScrollVirtualOffsetY(nScrollPosY);

    //�ؼ������Ͻ�����ֵ
    ui::UiPoint ptTile(rc.left, rc.top - yOffset);

    size_t iCount = 0;
    //��һ��Ԫ���Ǳ�ͷ�ؼ�������������ݣ����Դ�1��ʼ
    for (size_t index = 1; index < nItemCount; ++index) {
        Control* pControl = pDataView->GetItemAt(index);
        if (pControl == nullptr) {
            continue;
        }
        //��ǰ����Ԫ�ص�������
        const size_t nElementIndex = nTopElementIndex + iCount;

        //���õ�ǰ�ؼ��Ĵ�С��λ��
        ui::UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + cx, ptTile.y + cy);
        pControl->SetPos(rcTile);

        // �������        
        if (nElementIndex < pDataView->GetElementCount()) {
            if (!pControl->IsVisible()) {
                pControl->SetVisible(true);
            }
            pDataView->FillElement(pControl, nElementIndex);
            diplayItemIndexList.push_back(nElementIndex);
        }
        else {
            if (pControl->IsVisible()) {
                pControl->SetVisible(false);
            }
        }

        ptTile.y += cy + GetChildMarginY();
        ++iCount;
    }
    pDataView->SetDisplayDataItems(diplayItemIndexList);
    pDataView->OnArrangeChild();
}

size_t ListCtrlDataLayout::AjustMaxItem(UiRect rc) const
{
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return 1;
    }
    int32_t nItemHeight = GetItemHeight();
    ASSERT(nItemHeight > 0);
    if (nItemHeight <= 0) {
        return 1;
    }
    if (rc.IsEmpty()) {
        return 1;
    }
    int32_t nRows = 0;
    int32_t nHeaderHeight = GetHeaderHeight();
    if (nHeaderHeight > 0) {
        nRows += 1;
        rc.top += nHeaderHeight;
        rc.Validate();
    }

    nRows += rc.Height() / (nItemHeight + GetChildMarginY() / 2);
    //��֤������
    if (nRows > 1) {
        int32_t calcHeight = nRows * nItemHeight + (nRows - 1) * GetChildMarginY();
        if (calcHeight < rc.Height()) {
            nRows += 1;
        }
    }
    
    if (!pDataView->IsNormalMode()) {
        //�Ǳ�׼ģʽ����Ҫ���Ӽ���õ����
        int64_t nScrollPosY = pDataView->GetScrollPos().cy;
        int32_t nCalcRows = pDataView->GetMaxDataItemsToShow(nScrollPosY, rc.Height());
        if (nCalcRows > 0) {
            nCalcRows += 1;
            if (nCalcRows > nRows) {
                nRows = nCalcRows;
            }
        }
    }

    //��������1�У�ȷ����ʵ�ؼ��������������ʾ����
    nRows += 1;
    return nRows;
}

size_t ListCtrlDataLayout::GetTopElementIndex(UiRect /*rc*/) const
{
    size_t nTopElementIndex = 0;
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return nTopElementIndex;
    }
    int64_t nScrollPosY = pDataView->GetScrollPos().cy;
    if (!pDataView->IsNormalMode()) {
        //�Ǳ�׼ģʽ
        nTopElementIndex = pDataView->GetTopDataItemIndex(nScrollPosY);
    }
    else {
        int32_t nItemHeight = GetItemHeight();
        ASSERT(nItemHeight > 0);
        if (nItemHeight > 0) {
            nTopElementIndex = static_cast<size_t>(nScrollPosY / nItemHeight);
        }
    }
    return nTopElementIndex;
}

bool ListCtrlDataLayout::IsElementDisplay(UiRect rc, size_t iIndex) const
{
    if (!Box::IsValidItemIndex(iIndex)) {
        return false;
    }
    std::vector<size_t> itemIndexList;
    GetDisplayElements(rc, itemIndexList);
    return std::find(itemIndexList.begin(), itemIndexList.end(), iIndex) != itemIndexList.end();
}

void ListCtrlDataLayout::GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const
{
    GetDisplayElements(rc, collection, nullptr);
}

void ListCtrlDataLayout::GetDisplayElements(UiRect rc, std::vector<size_t>& collection,
                                            std::vector<size_t>* pAtTopItemIndexList) const
{
    collection.clear();
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return;
    }
    if (pDataView->GetItemCount() <= 1) {
        return;
    }
    int64_t nScrollPosY = pDataView->GetScrollPos().cy;
    if (!pDataView->IsNormalMode()) {
        //�Ǳ�׼ģʽ
        pDataView->GetMaxDataItemsToShow(nScrollPosY, rc.Height(), 
                                         &collection, pAtTopItemIndexList);
        return;
    }

    int32_t nItemHeight = GetItemHeight();
    ASSERT(nItemHeight >= 0);
    if (nItemHeight <= 0) {
        return;
    }
    rc.top += GetHeaderHeight();
    rc.Validate();

    size_t min = (size_t)(nScrollPosY / nItemHeight);
    size_t max = min + (size_t)(rc.Height() / nItemHeight);
    size_t nCount = pDataView->GetElementCount();
    if (nCount > 0) {
        if (max >= nCount) {
            max = nCount - 1;
        }
    }
    else {
        return;
    }
    for (size_t i = min; i <= max; ++i) {
        collection.push_back(i);
    }
}

bool ListCtrlDataLayout::NeedReArrange() const
{
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return false;
    }
    size_t nItemCount = pDataView->GetItemCount();//�������ʾ�ؼ��������У�
    if (nItemCount == 0) {
        return false;
    }

    if (pDataView->GetElementCount() <= nItemCount) {
        return false;
    }

    UiRect rcThis = pDataView->GetPos();
    if (rcThis.IsEmpty()) {
        return false;
    }

    int64_t nScrollPosY = pDataView->GetScrollPos().cy;//�¹�����λ��
    int64_t nVirtualOffsetY = pDataView->GetScrollVirtualOffset().cy;//ԭ������λ��
    //ֻҪ����λ�÷����仯������Ҫ���²���
    return (nScrollPosY != nVirtualOffsetY);
}

void ListCtrlDataLayout::EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const
{
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return;
    }
    if (!Box::IsValidItemIndex(iIndex) || (iIndex >= pDataView->GetElementCount())) {
        return;
    }
    ScrollBar* pVScrollBar = pDataView->GetVScrollBar();
    if (pVScrollBar == nullptr) {
        return;
    }
    if (!bToTop) {
        std::vector<size_t> atTopItemIndexList;
        std::vector<size_t> itemIndexList;
        GetDisplayElements(rc, itemIndexList, &atTopItemIndexList);
        bool bDisplay = std::find(itemIndexList.begin(), itemIndexList.end(), iIndex) != itemIndexList.end();
        bool bFirst = false;
        bool bLast = false;
        if (!itemIndexList.empty()) {
            for (size_t i = 0; i < itemIndexList.size(); ++i) {
                if (std::find(atTopItemIndexList.begin(), 
                              atTopItemIndexList.end(), 
                              itemIndexList[i]) == atTopItemIndexList.end()) {
                    //��һ�����ö�������
                    bFirst = itemIndexList[i] == iIndex;
                    break;
                }
            }            
            bLast = itemIndexList[itemIndexList.size() - 1] == iIndex;
        }
        if (bDisplay && !bLast && !bFirst) {
            //�Ѿ�����ʾ״̬
            return;
        }
    }
    UiSize szElementSize = GetElementSize(0, iIndex); //Ŀ��Ԫ�صĴ�С
    int64_t nNewTopPos = 0;     //��������ʱ��λ��
    int64_t nNewBottomPos = 0;  //�ײ�����ʱ��λ��
    if (iIndex > 0) {
        nNewTopPos = GetElementsHeight(iIndex, false);
        if (!bToTop) {
            //�ײ�����
            nNewBottomPos = GetElementsHeight(iIndex, false);
            int64_t nNewPosWithTop = GetElementsHeight(iIndex, true);
            int64_t nTopHeights = GetHeaderHeight();
            if (nNewPosWithTop > nNewBottomPos) {
                nTopHeights += (nNewPosWithTop - nNewBottomPos);
            }
            //�۳��ö���ĸ߶ȡ�Header�ĸ߶Ⱥ������߶�
            nNewBottomPos -= rc.Height();
            nNewBottomPos += nTopHeights;
            nNewBottomPos += szElementSize.cy;
        }
    }

    if (nNewTopPos < 0) {
        nNewTopPos = 0;
    }
    if (nNewTopPos > pVScrollBar->GetScrollRange()) {
        nNewTopPos = pVScrollBar->GetScrollRange();
    }
    if (nNewBottomPos < 0) {
        nNewBottomPos = 0;
    }
    if (nNewBottomPos > pVScrollBar->GetScrollRange()) {
        nNewBottomPos = pVScrollBar->GetScrollRange();
    }
    ui::UiSize64 sz = pDataView->GetScrollPos();
    int64_t nNewPos = sz.cy;
    if (bToTop) {
        //��������
        nNewPos = nNewTopPos;
    }
    else {
        //δָ�����룬�����ж�
        int64_t diff = sz.cy - nNewBottomPos;
        if (diff < 0) {
            //���Ϲ������ײ�����
            nNewPos = nNewBottomPos;
        }
        else {
            //���¹�������������
            nNewPos = nNewTopPos;
        }
    }
    sz.cy = nNewPos;
    pDataView->SetScrollPos(sz);
}

int64_t ListCtrlDataLayout::GetElementsHeight(size_t nCount, bool bIncludeAtTops) const
{
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return 0;
    }
    if (!Box::IsValidItemIndex(nCount)) {
        nCount = pDataView->GetElementCount();
    }
    if ((nCount == 0) || !Box::IsValidItemIndex(nCount)){
        return 0;
    }
    int64_t nTotalHeight = 0;
    if (!pDataView->IsNormalMode()) {
        //�Ǳ�׼ģʽ
        nTotalHeight = pDataView->GetDataItemTotalHeights(nCount, bIncludeAtTops);
    }
    else {
        int32_t nItemHeight = GetItemHeight();
        ASSERT(nItemHeight > 0);
        if (nItemHeight <= 0) {
            return 0;
        }
        if (nCount <= 1) {
            //ֻ��1��
            nTotalHeight = nItemHeight + GetChildMarginY();
        }
        else {
            int64_t iChildMargin = 0;
            if (GetChildMarginY() > 0) {
                iChildMargin = GetChildMarginY();
            }
            int64_t childMarginTotal = ((int64_t)nCount - 1) * iChildMargin;
            nTotalHeight = nItemHeight * nCount + childMarginTotal;
        }
    }
    return nTotalHeight;
}

UiSize ListCtrlDataLayout::GetElementSize(int32_t rcWidth, size_t nElementIndex) const
{
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return UiSize();
    }
    UiSize szElementSize;
    szElementSize.cx = std::max(GetItemWidth(), rcWidth);
    if (!pDataView->IsNormalMode()) {
        //�Ǳ�׼ģʽ���и߿��ܸ�����ͬ
        szElementSize.cy = pDataView->GetDataItemHeight(nElementIndex);
    }
    else {
        //��׼ģʽ�������еĸ߶ȶ���ͬ��
        szElementSize.cy = GetItemHeight();
    }    
    return szElementSize;
}

int32_t ListCtrlDataLayout::GetItemWidth() const
{
    //�������ͷ�Ŀ�����ͬ
    int32_t nItemWidth = 0;
    ListCtrlDataView* pDataView = GetDataView();
    if (pDataView != nullptr) {
        nItemWidth = pDataView->GetListCtrlWidth();
    }
    return nItemWidth;
}

int32_t ListCtrlDataLayout::GetItemHeight() const
{
    //�����еĸ߶���ͬ�����Ҵ����ö�ȡ
    int32_t nItemHeight = 0;
    ListCtrlDataView* pDataView = GetDataView();
    if ((pDataView != nullptr) && (pDataView->m_pListCtrl != nullptr)) {
        nItemHeight = pDataView->m_pListCtrl->GetDataItemHeight();
    }
    return nItemHeight;
}

int32_t ListCtrlDataLayout::GetHeaderHeight() const
{
    int32_t nHeaderHeight = 0;
    ListCtrlDataView* pDataView = GetDataView();
    if (pDataView != nullptr) {
        Control* pHeaderCtrl = nullptr;
        size_t nItemCount = pDataView->GetItemCount();
        if (nItemCount > 0) {
            //��һ��Ԫ���Ǳ�ͷ�ؼ���������λ�ô�С
            pHeaderCtrl = pDataView->GetItemAt(0);            
        }
        if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
            nHeaderHeight = pHeaderCtrl->GetFixedHeight().GetInt32();
        }
    }
    return nHeaderHeight;
}

}//namespace ui
