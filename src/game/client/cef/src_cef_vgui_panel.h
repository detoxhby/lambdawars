//====== Copyright � Sandern Corporation, All rights reserved. ===========//
//
// Purpose: 
//
//=============================================================================

#ifndef SRC_CEF_VGUI_PANEL_H
#define SRC_CEF_VGUI_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include "materialsystem/MaterialSystemUtil.h"

class SrcCefBrowser;
class CCefTextureGenerator;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class SrcCefVGUIPanel : public vgui::Panel 
{
public:
	DECLARE_CLASS_SIMPLE(SrcCefVGUIPanel, vgui::Panel);

	SrcCefVGUIPanel( const char *pName, SrcCefBrowser *pController, vgui::Panel *pParent );
	~SrcCefVGUIPanel();

	virtual bool ResizeTexture( int width, int height );
	virtual void MarkTextureDirty( int dirtyx, int dirtyy, int dirtyxend, int dirtyyend );

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnThink();
	virtual void Paint();
	virtual void OnSizeChanged(int newWide, int newTall);
	virtual void PerformLayout();

	virtual void DrawWebview();

	int	GetEventFlags();

	virtual void OnCursorEntered();
	virtual void OnCursorExited();
	virtual void OnCursorMoved(int x,int y);
	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void OnMouseDoublePressed(vgui::MouseCode code);
	virtual void OnMouseReleased(vgui::MouseCode code);
	virtual void OnMouseWheeled(int delta);

	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	virtual void OnKeyTyped(wchar_t unichar);
	virtual void OnKeyCodeReleased(vgui::KeyCode code);

	virtual vgui::HCursor GetCursor();

	// Hack for working nice with VGUI input
	// Should be removed once we mix with VGUI anymore
	int GetTextureID();
	float GetTexS1();
	float GetTexT1();
	void SetDoNotDraw( bool state );
	bool GetDoNotDraw( void );

protected:
	int	GetBrowserID();

private:
	virtual void UpdatePressedParent( vgui::MouseCode code, bool state );
	virtual bool IsPressedParent( vgui::MouseCode code );

private:
	int m_iMouseX, m_iMouseY;
	int m_iEventFlags;
	SrcCefBrowser *m_pBrowser;

	bool m_bCalledLeftPressedParent;
	bool m_bCalledRightPressedParent;
	bool m_bCalledMiddlePressedParent;

	// Texture variables
	bool m_bSizeChanged;
	CTextureReference	m_RenderBuffer;
	CCefTextureGenerator *m_pTextureRegen;
	CMaterialReference m_MatRef;
	char m_MatWebViewName[MAX_PATH];
	char m_TextureWebViewName[MAX_PATH];

	vgui::HFont m_hLoadingFont;

	int m_iTextureID;
	int m_iWVWide, m_iWVTall;
	int m_iTexWide, m_iTexTall;
	Color m_Color;
	float m_fTexS1, m_fTexT1;

	int m_iDirtyX, m_iDirtyY, m_iDirtyXEnd, m_iDirtyYEnd;

	// Hack for working nice with VGUI input
	int m_iTopZPos, m_iBottomZPos;
	bool m_bDontDraw;
};

// Hack for working nice with VGUI input
inline int SrcCefVGUIPanel::GetTextureID()
{
	return m_iTextureID;
}

inline float SrcCefVGUIPanel::GetTexS1()
{
	return m_fTexS1;
}

inline float SrcCefVGUIPanel::GetTexT1()
{
	return m_fTexT1;
}

inline void SrcCefVGUIPanel::SetDoNotDraw( bool state )
{
	m_bDontDraw = state;
}

inline bool SrcCefVGUIPanel::GetDoNotDraw( void )
{
	return m_bDontDraw;
}

#endif // SRC_CEF_VGUI_PANEL_H