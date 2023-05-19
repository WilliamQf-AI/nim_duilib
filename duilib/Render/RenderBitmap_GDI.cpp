 #include "RenderBitmap_GDI.h"

namespace ui
{

HBITMAP RenderBitmap_GDI::CreateDIBBitmap(HDC hdc, int width, int height, LPVOID* pBits)
{
	if (hdc == NULL || width <= 0 || height <= 0)
		return NULL;

	BITMAPINFO bmi = { 0 };
	::ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;//������ʾλͼʱ���ϵ��·���ģ����Ͻ�ΪԲ��
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = width * height * sizeof(DWORD);

	return ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, pBits, NULL, 0);
}

RenderBitmap_GDI::RenderBitmap_GDI() 
{
	m_hBitmap = NULL;
	CleanUp();
}

RenderBitmap_GDI::~RenderBitmap_GDI()
{
	CleanUp();
}

void RenderBitmap_GDI::CleanUp()
{
	if (m_hBitmap != NULL)
	{
		::DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}

	m_pPiexl = NULL;
	m_nWidth = m_nHeight = 0;
}

bool RenderBitmap_GDI::Init(HDC hSrcDC, int width, int height)
{
	CleanUp();

	LPVOID pBmpBits = NULL;
	m_hBitmap = CreateDIBBitmap(hSrcDC, width, height, &pBmpBits);
	if (m_hBitmap != NULL)
	{
		m_nWidth = width;
		m_nHeight = height;

		BITMAP info;
		::GetObject(m_hBitmap, sizeof(info), &info);
		m_pPiexl = LPBYTE(info.bmBits);
		return true;
	}
	else
	{
		//DWORD err = GetLastError();
		ASSERT(FALSE);
		return false;
	}
}

void RenderBitmap_GDI::Clear()
{
	ASSERT(m_hBitmap && m_pPiexl != NULL);
	if (m_pPiexl == NULL)
		return;

	::memset(m_pPiexl, 0, m_nWidth * m_nHeight * 4);
}

HBITMAP RenderBitmap_GDI::DetachBitmap()
{
	HBITMAP hBitmap = m_hBitmap;
	m_hBitmap = NULL;
	CleanUp();

	return hBitmap;
}

int RenderBitmap_GDI::GetWidth()
{
	return m_nWidth;
}

int RenderBitmap_GDI::GetHeight()
{
	return m_nHeight;
}

void RenderBitmap_GDI::ClearAlpha(const UiRect& rcDirty, int alpha)
{
	ASSERT(m_hBitmap && m_pPiexl != NULL);
	if (m_pPiexl == NULL)
		return;

	unsigned int * pBmpBits = (unsigned int *)m_pPiexl;
	int nTop = std::max((int)rcDirty.top, 0);
	int nBottom = std::min((int)rcDirty.bottom, m_nHeight);
	int nLeft = std::max((int)rcDirty.left, 0);
	int nRight = std::min((int)rcDirty.right, m_nWidth);
	if (nRight > nLeft)	{
		for (int i = nTop; i < nBottom; ++i) {
			::memset(pBmpBits + i * m_nWidth + nLeft, alpha, (nRight - nLeft) * 4);
		}
	}
}

void RenderBitmap_GDI::RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding, int alpha)
{
	// �˺���������GDI��API��Ⱦλͼ�����¶�ʧalphaͨ������������԰�alphaͨ��������
	// ������Ⱦλͼʱ������GDI+��AlphaBlend��API��λͼ�����˰�͸����alphaͨ��ʱ�����ܵ���û����ȷ������alphaͨ��
	ASSERT(m_hBitmap && m_pPiexl != NULL);
	if (m_pPiexl == NULL)
		return;

	unsigned int * pBmpBits = (unsigned int *)m_pPiexl;
	int nTop = std::max((int)rcDirty.top, 0);
	int nBottom = std::min((int)rcDirty.bottom, m_nHeight);
	int nLeft = std::max((int)rcDirty.left, 0);
	int nRight = std::min((int)rcDirty.right, m_nWidth);

	for (int i = nTop; i < nBottom; i++) {
		for (int j = nLeft; j < nRight; j++) {
			BYTE* a = (BYTE*)(pBmpBits + i * m_nWidth + j) + 3;

			if (((j >= rcShadowPadding.left && j < m_nWidth - rcShadowPadding.right)
				|| (i >= rcShadowPadding.top && i < m_nHeight - rcShadowPadding.bottom))) {

				// ClearAlphaʱ����alphaͨ������Ϊĳ��ֵ
				// �����ֵû�б仯����֤������û�л����κ����ݣ���alpha��Ϊ0
				if (alpha != 0 && *a == alpha)
					*a = 0;
				// �����ֵ��Ϊ0����֤�����汻����DrawText��GDI�������ƹ�����alpha����Ϊ0����ʱalpha��Ϊ255
				else if (*a == 0)
					*a = 255;
			}
		}
	}
}

void RenderBitmap_GDI::RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding)
{
	// ����ʲô��������Ѵ������alphaaͨ������Ϊ255
	ASSERT(m_hBitmap && m_pPiexl != NULL);
	if (m_pPiexl == NULL)
		return;

	unsigned int * pBmpBits = (unsigned int *)m_pPiexl;
	int nTop = std::max((int)rcDirty.top, 0);
	int nBottom = std::min((int)rcDirty.bottom, m_nHeight);
	int nLeft = std::max((int)rcDirty.left, 0);
	int nRight = std::min((int)rcDirty.right, m_nWidth);

	for (int i = nTop; i < nBottom; i++) {
		for (int j = nLeft; j < nRight; j++) {
			BYTE* a = (BYTE*)(pBmpBits + i * m_nWidth + j) + 3;

			if (((j >= rcShadowPadding.left && j < m_nWidth - rcShadowPadding.right)
				|| (i >= rcShadowPadding.top && i < m_nHeight - rcShadowPadding.bottom))) {
					*a = 255;
			}
		}
	}
}

HBITMAP RenderBitmap_GDI::GetHBitmap()
{
	ASSERT(m_hBitmap != nullptr);
	return m_hBitmap;
}

}