//====== Copyright � Sandern Corporation, All rights reserved. ===========//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "src_cef_browser.h"
#include "src_cef.h"
#include "src_cef_vgui_panel.h"
#include "src_cef_osrenderer.h"
#include "src_cef_js.h"

#ifdef ENABLE_PYTHON
	#include "srcpy.h"
	#include "src_cef_python.h"
#endif // ENABLE_PYTHON

#include "inputsystem/iinputsystem.h"
#include <vgui/IInput.h>
#include "materialsystem/materialsystem_config.h"
#include "steam/steam_api.h"

#ifdef WIN32
#include <winlite.h>
#include <ShellAPI.h>
#endif // WIN32

// CEF
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"
#include "include/cef_client.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Small helper function for opening urls
//-----------------------------------------------------------------------------
static void OpenURL( const char *url )
{
	// Open url in steam browser if fullscreen
	// If windowed, just do a shell execute so it executes in the default browser
#ifdef WIN32
	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
	if( config.Windowed() )
	{
		ShellExecute(NULL, "open", url,
						NULL, NULL, SW_SHOWNORMAL);
	}
	else
#endif // WIN32
	{
		if( !steamapicontext || !steamapicontext->SteamFriends() )
		{
			Warning("OpenURL: could not get steam api context!\n");
			return;
		}
		steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage( url );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Cef browser internal implementation
//-----------------------------------------------------------------------------
class CefClientHandler : public CefClient,
                      public CefContextMenuHandler,
                      public CefDisplayHandler,
                      public CefDownloadHandler,
                      public CefGeolocationHandler,
                      public CefKeyboardHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRequestHandler
{
public:
	CefClientHandler( SrcCefBrowser *pSrcBrowser );

	CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }
	int GetBrowserId() { return m_BrowserId; }

	virtual void Destroy();

	// CefClient methods
	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() {
		return this;
	}
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() {
		return this;
	}
	virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() {
		return this;
	}
	virtual CefRefPtr<CefGeolocationHandler> GetGeolocationHandler() {
		return this;
	}
	virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() {
		return this;
	}
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() {
		return this;
	}
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() {
		return this;
	}
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() {
		return m_OSRHandler.get();
	}
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() {
		return this;
	}

	// Client
	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
										CefProcessId source_process,
										CefRefPtr<CefProcessMessage> message);

	// CefContextMenuHandler methods
	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
									CefRefPtr<CefFrame> frame,
									CefRefPtr<CefContextMenuParams> params,
									CefRefPtr<CefMenuModel> model);

	// CefDisplayHandler methods
	virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
								const CefString& message,
								const CefString& source,
								int line);

	// CefDownloadHandler methods
	virtual void OnBeforeDownload(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefDownloadItem> download_item,
		const CefString& suggested_name,
		CefRefPtr<CefBeforeDownloadCallback> callback) {}

	// CefLifeSpanHandler methods
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser);

	virtual bool DoClose(CefRefPtr<CefBrowser> browser);

	virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
										TerminationStatus status);

	// CefLoadHandler methods
	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame);
	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode);
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
							const CefString& errorText, const CefString& failedUrl);

	virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
									bool isLoading,
									bool canGoBack,
									bool canGoForward);

	// CefRequestHandler
	virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								CefRefPtr<CefRequest> request,
								bool is_redirect);

	virtual void OnResourceRedirect(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								const CefString& old_url,
								CefString& new_url);

	// CefRenderHandler
	void SetOSRHandler(CefRefPtr<SrcCefOSRRenderer> handler) {
		m_OSRHandler = handler;
	}
	CefRefPtr<SrcCefOSRRenderer> GetOSRHandler() { return m_OSRHandler; }

	// Misc
	bool IsInitialized() { return m_bInitialized; }
	float GetLastPingTime() { return m_fLastPingTime; }

private:
	// The child browser window
	CefRefPtr<CefBrowser> m_Browser;
	CefRefPtr<SrcCefOSRRenderer> m_OSRHandler;

	// The child browser id
	int m_BrowserId;

	// Tests if "OnAfterCreated" is called. This means the browser/webview is "initialized"
	bool m_bInitialized;
	// Last time we received a pong from the browser process
	float m_fLastPingTime;

	// Internal
	HWND m_hostWindow;
	SrcCefBrowser *m_pSrcBrowser;

	IMPLEMENT_REFCOUNTING( CefClientHandler );
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CefClientHandler::CefClientHandler( SrcCefBrowser *pSrcBrowser ) : m_BrowserId(0), m_pSrcBrowser( pSrcBrowser ), 
	m_OSRHandler( NULL ), m_bInitialized(false), m_fLastPingTime(-1)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CefClientHandler::Destroy()
{
	if( GetBrowser() )
	{
		GetBrowser()->GetHost()->CloseBrowser( true );
	}

	m_pSrcBrowser = NULL;
	SetOSRHandler( NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CefClientHandler::OnProcessMessageReceived(	CefRefPtr<CefBrowser> browser,
								CefProcessId source_process,
								CefRefPtr<CefProcessMessage> message )
{
	if( !m_pSrcBrowser )
		return false;

	if( message->GetName() == "pong" ) 
	{
		if( g_debug_cef.GetBool() )
			DevMsg( "Received PONG from render process of browser %d!\n", browser->GetIdentifier() );
		m_fLastPingTime = Plat_FloatTime();
		return true;
	}
	else if( message->GetName() == "methodcall" )
	{
		CefRefPtr<CefListValue> args = message->GetArgumentList();
		int iIdentifier = args->GetInt( 0 );
		CefRefPtr<CefListValue> methodargs = args->GetList( 1 );
	
		if( args->GetType( 2 ) == VTYPE_NULL )
		{
			m_pSrcBrowser->OnMethodCall( iIdentifier, methodargs );
		}
		else
		{
			int iCallbackID = args->GetInt( 2 );
			m_pSrcBrowser->OnMethodCall( iIdentifier, methodargs, &iCallbackID );
		}
		return true;
	}
	else if( message->GetName() == "oncontextcreated" )
	{
		m_pSrcBrowser->OnContextCreated();
	}
	else if( message->GetName() == "openurl" )
	{
		CefRefPtr<CefListValue> args = message->GetArgumentList();
		OpenURL( args->GetString( 0 ).ToString().c_str() );
	}
	else if( message->GetName() == "msg" )
	{
		CefRefPtr<CefListValue> args = message->GetArgumentList();
		Msg( "Browser %d Render Process: %ls", browser->GetIdentifier(), args->GetString( 0 ).c_str() );
		return true;
	}
	else if( message->GetName() == "warning" )
	{
		CefRefPtr<CefListValue> args = message->GetArgumentList();
		Warning( "Browser %d Render Process: %ls", browser->GetIdentifier(), args->GetString( 0 ).c_str() );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CefClientHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								CefRefPtr<CefContextMenuParams> params,
								CefRefPtr<CefMenuModel> model)
{
	// Always clear context menus
	model->Clear();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CefClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
							const CefString& message,
							const CefString& source,
							int line)
{
	Warning("%d %ls: %ls\n", line, source.c_str(), message.c_str() );
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CefClientHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
									TerminationStatus status)
{
	Warning("Browser %d render process crashed with status code %d!\n", browser->GetIdentifier(), status);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CefClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) 
{
	if( !m_pSrcBrowser )
		return;

	if( g_debug_cef.GetBool() )
		DevMsg( "#%d %s: SrcCefBrowser::OnAfterCreated\n", browser->GetIdentifier(), m_pSrcBrowser->GetName() );

	if( !m_Browser.get() ) 
	{
		// We need to keep the main child window, but not popup windows
		m_Browser = browser;
		m_BrowserId = browser->GetIdentifier();
	}

	m_bInitialized = true;
	m_pSrcBrowser->Ping();
	m_pSrcBrowser->OnAfterCreated();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CefClientHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
	if( browser->IsLoading() )
		browser->StopLoad();
	browser->GetHost()->ParentWindowWillClose();
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CefClientHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
	if( !m_pSrcBrowser )
		return;

	if( g_debug_cef.GetBool() )
		DevMsg( "#%d %s: SrcCefBrowser::OnLoadStart\n", browser->GetIdentifier(), m_pSrcBrowser->GetName() );

	m_pSrcBrowser->OnLoadStart( frame );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CefClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	if( !m_pSrcBrowser )
		return;

	if( g_debug_cef.GetBool() )
		DevMsg( "#%d %s: SrcCefBrowser::OnLoadEnd\n", browser->GetIdentifier(), m_pSrcBrowser->GetName() );

	m_pSrcBrowser->OnLoadEnd( frame, httpStatusCode );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CefClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
						const CefString& errorText, const CefString& failedUrl)
{
	if( !m_pSrcBrowser )
		return;
	m_pSrcBrowser->OnLoadError( frame, errorCode, errorText.c_str(), failedUrl.c_str() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CefClientHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
									bool isLoading,
									bool canGoBack,
									bool canGoForward)
{
	if( !m_pSrcBrowser )
		return;
	m_pSrcBrowser->OnLoadingStateChange( isLoading, canGoBack, canGoForward );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CefClientHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
							CefRefPtr<CefFrame> frame,
							CefRefPtr<CefRequest> request,
							bool is_redirect)
{
	if( !m_pSrcBrowser )
		return false;

	// Default behavior: allow navigating away
	bool bDenyNavigation = false;

	// Always allow sub frames to navigate to anything
	if( frame->IsMain() )
	{
		// Always allow dev tools urls
		std::string url = request->GetURL().ToString();
		std::string chromedevtoolspro( "chrome-devtools://");
		if( url.compare( 0, chromedevtoolspro.size(), chromedevtoolspro ) != 0 )
		{
			if( m_pSrcBrowser->GetNavigationBehavior() == SrcCefBrowser::NT_PREVENTALL )
			{
				// This mode prevents from navigating away from the current page
				if( browser->GetMainFrame()->GetURL() != request->GetURL() )
					bDenyNavigation = true;
			}
			else if( m_pSrcBrowser->GetNavigationBehavior() == SrcCefBrowser::NT_ONLYFILEPROT )
			{
				// This mode only allows navigating to urls starting with the file protocol
				std::string filepro( "file://");

				if( url.compare( 0, filepro.size(), filepro ) )
					bDenyNavigation = true;
			}
		}

		// If we don't allow navigation, open the url in a new window
		if( bDenyNavigation )
		{
			OpenURL( request->GetURL().ToString().c_str() );
		}
	}

	return bDenyNavigation;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CefClientHandler::OnResourceRedirect(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								const CefString& old_url,
								CefString& new_url)
{
}

//-----------------------------------------------------------------------------
// Purpose: Cef browser
//-----------------------------------------------------------------------------
SrcCefBrowser::SrcCefBrowser( const char *name, const char *pURL ) : m_bPerformLayout(true), m_bVisible(false), m_pPanel(NULL),
	m_bGameInputEnabled(false), m_bUseMouseCapture(false), m_bPassMouseTruIfAlphaZero(false), m_bHasFocus(false), m_CefClientHandler(NULL),
	m_bInitializePingSuccessful(false), m_bWasHidden(false)
{
	m_Name = name ? name : "UnknownCefBrowser";

	// Create panel and texture generator
	m_pPanel = new SrcCefVGUIPanel( name, this, NULL );

	// Initialize browser
	CEFSystem().AddBrowser( this );

	if( !CEFSystem().IsRunning() )
	{
		Warning("CEFSystem not running, not creating browser\n");
		return;
	}

	m_URL = pURL ? pURL : "";
	m_CefClientHandler = new CefClientHandler( this );

    CefWindowInfo info;
	info.SetAsOffScreen( 0 );
	info.SetTransparentPainting( TRUE );

	m_CefClientHandler->SetOSRHandler( new SrcCefOSRRenderer( this, true ) );
	
	// Browser settings
    CefBrowserSettings settings;
	settings.web_security = STATE_DISABLED;

    // Creat the new child browser window
	if( g_debug_cef.GetBool() )
		DevMsg( "%s: CefBrowserHost::CreateBrowser\n", m_Name.c_str() );
	
	m_fBrowserCreateTime = Plat_FloatTime();
	if( !CefBrowserHost::CreateBrowser( info, m_CefClientHandler, m_URL, settings, NULL ) )
	{
		Warning(" Failed to create CEF browser %s\n", name );
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
SrcCefBrowser::~SrcCefBrowser()
{
	// Remove ourself from the list
	CEFSystem().RemoveBrowser( this );

	Destroy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::Destroy( void )
{
	// Delete panel
	if( m_pPanel )
	{
		delete m_pPanel;
		m_pPanel = NULL;
	}

	// Close browser
	if( m_CefClientHandler )
	{
		if( g_debug_cef.GetBool() )
			DevMsg( "#%d %s: SrcCefBrowser::Destroy\n", GetBrowser() ? GetBrowser()->GetIdentifier() : -1, m_Name.c_str() );

		m_CefClientHandler->Destroy();
		m_CefClientHandler = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool SrcCefBrowser::IsValid( void )
{
	return m_CefClientHandler && m_CefClientHandler->GetBrowser();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CefRefPtr<SrcCefOSRRenderer> SrcCefBrowser::GetOSRHandler()
{
	return m_CefClientHandler->GetOSRHandler();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CefRefPtr<CefBrowser> SrcCefBrowser::GetBrowser()
{
	return m_CefClientHandler->GetBrowser();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::Think( void )
{
	if( m_bPerformLayout )
	{
		PerformLayout();
		m_CefClientHandler->GetBrowser()->GetHost()->WasResized();

		m_bPerformLayout = false;
	}

	// Tell CEF not to paint if panel is hidden
	bool bFullyVisible = GetPanel()->IsFullyVisible();
	if( bFullyVisible && m_bWasHidden )
	{
		WasHidden( false );
		m_bWasHidden = false;
	}
	else if( !bFullyVisible && !m_bWasHidden )
	{
		WasHidden( true );
		m_bWasHidden = true;
	}

	if( !m_bInitializePingSuccessful )
	{
		if( !m_CefClientHandler->IsInitialized() || m_CefClientHandler->GetLastPingTime() == -1 )
		{
			float fLifeTime = Plat_FloatTime() - m_fBrowserCreateTime;
			if( fLifeTime > 15.0f )
			{
				Error( "Could not launch browser helper process. \n\n"
						"This is usually caused by security software blocking the process from launching. "
						"Please check your security software and unblock \"lambdawars_browser.exe\". \n\n"
						"Your security software might also temporary block the process for scanning. "
						"In this case just relaunch the game. "
				);
			}
		}
		else
		{
			m_bInitializePingSuccessful = true;
			if( g_debug_cef.GetBool() )
				DevMsg( "#%d %s: Browser creation time: %f\n", GetBrowser() ? GetBrowser()->GetIdentifier() : -1, m_Name.c_str(), Plat_FloatTime() - m_fBrowserCreateTime );
		}
	}

	vgui::VPANEL focus = vgui::input()->GetFocus();
	vgui::Panel *pPanel = GetPanel();
	if( pPanel->IsVisible() && focus == 0 )
	{
		if( !m_bHasFocus )
		{
			m_bHasFocus = true;
			GetBrowser()->GetHost()->SendFocusEvent(m_bHasFocus);
		}
	}
	else
	{
		if( m_bHasFocus )
		{
			m_bHasFocus = false;
			GetBrowser()->GetHost()->SendFocusEvent(m_bHasFocus);
		}
	}

	OnThink();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::OnAfterCreated( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::OnLoadStart( CefRefPtr<CefFrame> frame )
{
#ifdef ENABLE_PYTHON
	if( !SrcPySystem()->IsPythonRunning() )
		return;

	try
	{
		PyOnLoadStart( boost::python::object( PyCefFrame( frame ) ) );
	}
	catch( boost::python::error_already_set & )
	{
		PyErr_Print();
	}
#endif // ENABLE_PYTHON
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::OnLoadEnd( CefRefPtr<CefFrame> frame, int httpStatusCode )
{
#ifdef ENABLE_PYTHON
	if( !SrcPySystem()->IsPythonRunning() )
		return;

	try
	{
		PyOnLoadEnd( boost::python::object( PyCefFrame( frame ) ), httpStatusCode );
	}
	catch( boost::python::error_already_set & )
	{
		PyErr_Print();
	}
#endif // ENABLE_PYTHON
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::OnLoadError( CefRefPtr<CefFrame> frame, int errorCode, const wchar_t *errorText, const wchar_t *failedUrl )
{
#ifdef ENABLE_PYTHON
	if( !SrcPySystem()->IsPythonRunning() )
		return;

	try
	{
		// Convert error texts to Python unicode objects
		boost::python::object pyErrorText;
		if( errorText )
		{
			PyObject* pPyErrorText = PyUnicode_FromWideChar( errorText, wcslen(errorText) );
			if( pPyErrorText )
				pyErrorText = boost::python::object( boost::python::handle<>( pPyErrorText ) );
		}
		boost::python::object pyFailedURL;
		if( failedUrl )
		{
			PyObject* pPyFailedURL = PyUnicode_FromWideChar( failedUrl, wcslen(failedUrl) );
			if( pPyFailedURL )
				pyFailedURL = boost::python::object( boost::python::handle<>( pPyFailedURL ) );
		}

		PyOnLoadError( boost::python::object( PyCefFrame( frame ) ), errorCode, pyErrorText, pyFailedURL );
	}
	catch( boost::python::error_already_set & )
	{
		PyErr_Print();
	}
#endif // ENABLE_PYTHON
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::PerformLayout( void )
{
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::InvalidateLayout( void )
{
	m_bPerformLayout = true;
}

// Usage functions
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::SetSize( int wide, int tall )
{
	if( !IsValid() )
		return;

	m_pPanel->SetSize( wide, tall );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::SetPos( int x, int y )
{
	if( !IsValid() )
		return;

	m_pPanel->SetPos( x, y );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::SetZPos( int z )
{
	if( !IsValid() )
		return;

	m_pPanel->SetZPos( z );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::SetVisible( bool state )
{
	if( !IsValid() )
		return;

	if( m_bVisible == state )
		return;

	m_bVisible = state;
	m_pPanel->SetVisible( state );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool SrcCefBrowser::IsVisible()
{
	if( !IsValid() )
		return false;

	return m_pPanel->IsVisible(); 
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::SetMouseInputEnabled( bool state )
{
	if( !IsValid() )
	{
		Warning( "#%d %s: SrcCefBrowser::SetMouseInputEnabled: browser not valid yet!\n", GetBrowser() ? GetBrowser()->GetIdentifier() : -1, m_Name.c_str() );
		return;
	}

	m_pPanel->SetMouseInputEnabled( state );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::SetKeyBoardInputEnabled( bool state )
{
	if( !IsValid() )
	{
		Warning( "#%d %s: SrcCefBrowser::SetKeyBoardInputEnabled: browser not valid yet!\n", GetBrowser() ? GetBrowser()->GetIdentifier() : -1, m_Name.c_str() );
		return;
	}

	m_pPanel->SetKeyBoardInputEnabled( state );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool SrcCefBrowser::IsMouseInputEnabled()
{
	if( !IsValid() )
		return false;

	return m_pPanel->IsMouseInputEnabled( );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool SrcCefBrowser::IsKeyBoardInputEnabled()
{
	if( !IsValid() )
		return false;

	return m_pPanel->IsKeyBoardInputEnabled( );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int SrcCefBrowser::KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	if( !IsGameInputEnabled() )
		return 1;

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::SetCursor(vgui::HCursor cursor)
{
	if( !IsValid() )
		return;

	return m_pPanel->SetCursor( cursor );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
vgui::HCursor SrcCefBrowser::GetCursor()
{
	if( !IsValid() )
		return 0;

	return m_pPanel->GetCursor( );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int SrcCefBrowser::GetAlphaAt( int x, int y )
{
	if( !IsValid() )
		return 0;

	if( GetOSRHandler() )
		return GetOSRHandler()->GetAlphaAt( x, y );
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool SrcCefBrowser::IsLoading( void )
{
	if( !IsValid() )
		return false;

	return m_CefClientHandler->GetBrowser()->IsLoading();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::Reload( void )
{
	if( !IsValid() )
		return;

	m_CefClientHandler->GetBrowser()->Reload();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::ReloadIgnoreCache( void )
{
	if( !IsValid() )
		return;

	m_CefClientHandler->GetBrowser()->ReloadIgnoreCache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::StopLoad( void )
{
	if( !IsValid() )
		return;

	m_CefClientHandler->GetBrowser()->StopLoad();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::Focus()
{
	if( !IsValid() )
		return;
	m_CefClientHandler->GetBrowser()->GetHost()->SendFocusEvent( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::Unfocus()
{
	if( !IsValid() )
		return;
	m_CefClientHandler->GetBrowser()->GetHost()->SendFocusEvent( false );
}

//-----------------------------------------------------------------------------
// Purpose: Load an URL
//-----------------------------------------------------------------------------
void SrcCefBrowser::LoadURL( const char *url )
{
	if( !IsValid() )
		return;

	CefRefPtr<CefFrame> mainFrame = m_CefClientHandler->GetBrowser()->GetMainFrame();
	if( !mainFrame )
		return;

	mainFrame->LoadURL( CefString( url ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::WasHidden( bool hidden )
{
	if( !IsValid() )
		return;

	m_CefClientHandler->GetBrowser()->GetHost()->WasHidden( hidden );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SrcCefBrowser::SetNavigationBehavior( NavigationType behavior )
{
	if( !IsValid() )
		return;

	if( m_navigationBehavior == behavior )
		return;

	m_navigationBehavior = behavior; 
}

//-----------------------------------------------------------------------------
// Purpose: Execute javascript code
//-----------------------------------------------------------------------------
void SrcCefBrowser::ExecuteJavaScript( const char *code, const char *script_url, int start_line )
{
	if( !IsValid() )
		return;

	CefRefPtr<CefFrame> mainFrame = m_CefClientHandler->GetBrowser()->GetMainFrame();
	if( !mainFrame )
		return;

	mainFrame->ExecuteJavaScript( code, script_url, start_line );
}

//-----------------------------------------------------------------------------
// Purpose: Execute javascript code with reference to result
//-----------------------------------------------------------------------------
CefRefPtr<JSObject> SrcCefBrowser::ExecuteJavaScriptWithResult( const char *code, const char *script_url, int start_line )
{
	if( !IsValid() )
		return NULL;

	CefRefPtr<JSObject> jsObject = new JSObject();

	CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("calljswithresult");
	CefRefPtr<CefListValue> args = message->GetArgumentList();
	args->SetInt( 0, jsObject->GetIdentifier() );
	args->SetString( 1, code );
	GetBrowser()->SendProcessMessage(PID_RENDERER, message);
	return jsObject;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CefRefPtr<JSObject> SrcCefBrowser::CreateGlobalObject( const char *name )
{
	if( !IsValid() )
		return NULL;

	CefRefPtr<JSObject> jsObject = new JSObject( name );

	CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("createglobalobject");
	CefRefPtr<CefListValue> args = message->GetArgumentList();
	args->SetInt( 0, jsObject->GetIdentifier() );
	args->SetString( 1, name );
	GetBrowser()->SendProcessMessage(PID_RENDERER, message);
	return jsObject;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CefRefPtr<JSObject> SrcCefBrowser::CreateFunction( const char *name, CefRefPtr<JSObject> object, bool bHasCallback )
{
	if( !IsValid() )
		return NULL;

	CefRefPtr<JSObject> jsObject = new JSObject( name );

	CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create( !bHasCallback ? "createfunction" : "createfunctionwithcallback" );
	CefRefPtr<CefListValue> args = message->GetArgumentList();
	args->SetInt( 0, jsObject->GetIdentifier() );
	args->SetString( 1, name );
	if( object )
		args->SetInt( 2, object->GetIdentifier() );
	else
		args->SetNull( 2 );
	GetBrowser()->SendProcessMessage(PID_RENDERER, message);
	return jsObject;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::SendCallback( int *pCallbackID, CefRefPtr<CefListValue> methodargs )
{
	if( !IsValid() )
		return;

	if( !pCallbackID )
	{
		Warning("SendCallback: no callback specified\n");
		return;
	}

	CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("callbackmethod");
	CefRefPtr<CefListValue> args = message->GetArgumentList();
	args->SetInt( 0, *pCallbackID );
	args->SetList( 1, methodargs );

	GetBrowser()->SendProcessMessage(PID_RENDERER, message);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::Invoke( CefRefPtr<JSObject> object, const char *methodname,  CefRefPtr<CefListValue> methodargs )
{
	if( !IsValid() )
		return;

	CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("invoke");
	CefRefPtr<CefListValue> args = message->GetArgumentList();
	args->SetInt( 0, object ? object->GetIdentifier() : -1 );
	args->SetString( 1, methodname );
	args->SetList( 2, methodargs );

	GetBrowser()->SendProcessMessage(PID_RENDERER, message);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CefRefPtr<JSObject> SrcCefBrowser::InvokeWithResult( CefRefPtr<JSObject> object, const char *methodname,  CefRefPtr<CefListValue> methodargs )
{
	if( !IsValid() )
		return NULL;

	CefRefPtr<JSObject> jsResultObject = new JSObject();

	CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("invokewithresult");
	CefRefPtr<CefListValue> args = message->GetArgumentList();
	args->SetInt( 0, jsResultObject->GetIdentifier() );
	args->SetInt( 1, object ? object->GetIdentifier() : -1 );
	args->SetString( 2, methodname );
	args->SetList( 3, methodargs );

	GetBrowser()->SendProcessMessage(PID_RENDERER, message);

	return jsResultObject;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::OnMethodCall( int iIdentifier, CefRefPtr<CefListValue> methodargs, int *pCallbackID )
{
#ifdef ENABLE_PYTHON
	if( !SrcPySystem()->IsPythonRunning() )
		return;

	try
	{
		PyOnMethodCall( iIdentifier, CefValueListToPy( methodargs ), pCallbackID ? boost::python::object( *pCallbackID ) : boost::python::object() );
	}
	catch(boost::python::error_already_set &)
	{
		PyErr_Print();
	}
#endif // ENABLE_PYTHON
}

#ifdef ENABLE_PYTHON
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
boost::python::object SrcCefBrowser::PyGetMainFrame()
{
	if( !IsValid() )
		return boost::python::object();

	CefRefPtr<CefFrame> mainFrame = m_CefClientHandler->GetBrowser()->GetMainFrame();
	if( !mainFrame )
		return boost::python::object();

	return boost::python::object( PyCefFrame( mainFrame ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
boost::python::object SrcCefBrowser::PyCreateGlobalObject( const char *name )
{
	if( !IsValid() )
		return boost::python::object();

	CefRefPtr<JSObject> jsObject = CreateGlobalObject( name );
	if( !jsObject )
		return boost::python::object();

	return boost::python::object( PyJSObject( jsObject ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
boost::python::object SrcCefBrowser::PyCreateFunction( const char *name, PyJSObject *pPyObject, bool hascallback )
{
	if( !IsValid() || !pPyObject )
		return boost::python::object();

	CefRefPtr<JSObject> object = pPyObject->GetJSObject();
	if( !object )
		return boost::python::object();

	CefRefPtr<JSObject> jsObject = CreateFunction( name, object, hascallback );
	if( !jsObject )
		return boost::python::object();

	return boost::python::object( PyJSObject( jsObject ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
boost::python::object SrcCefBrowser::PyExecuteJavaScriptWithResult( const char *code, const char *script_url, int start_line )
{
	if( !IsValid() )
		return boost::python::object();

	CefRefPtr<JSObject> jsObject = ExecuteJavaScriptWithResult( code, script_url, start_line );
	if( !jsObject )
		return boost::python::object();

	return boost::python::object( PyJSObject( jsObject ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::PyInvoke( PyJSObject *object, const char *methodname, boost::python::list methodargs )
{
	if( !IsValid() )
		return;

	if( !methodname )
		return;

	Invoke( object ? object->GetJSObject() : NULL, methodname, PyToCefValueList( methodargs ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
boost::python::object SrcCefBrowser::PyInvokeWithResult( PyJSObject *object, const char *methodname, boost::python::list methodargs )
{
	if( !IsValid() )
		return boost::python::object();

	if( !methodname )
		return boost::python::object();

	CefRefPtr<JSObject> jsObject = InvokeWithResult( object ? object->GetJSObject() : NULL, methodname, PyToCefValueList( methodargs ) );
	if( !jsObject )
		return boost::python::object();

	return boost::python::object( PyJSObject( jsObject ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::PySendCallback( boost::python::object callbackid, boost::python::list pymethodargs )
{
	if( callbackid.ptr() == Py_None )
		return;

	int iCallbackID = boost::python::extract<int>( callbackid );

	// Convert list to CefListValue
	CefRefPtr<CefListValue> methodargs = PyToCefValueList( pymethodargs );

	// Do callback
	SendCallback( &iCallbackID, methodargs );
}
#endif // ENABLE_PYTHON

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::Ping()
{
	if( !IsValid() )
		return;

	CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("ping");
	CefRefPtr<CefListValue> args = message->GetArgumentList();
	CefRefPtr<CefListValue> val = CefListValue::Create();
	args->SetList(0, val);
	GetBrowser()->SendProcessMessage(PID_RENDERER, message);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SrcCefBrowser::ShowDevTools( ) 
{
	CefWindowInfo windowInfo;
	CefBrowserSettings settings;

#if defined(WIN32)
	windowInfo.SetAsPopup( GetBrowser()->GetHost()->GetWindowHandle(), "Lambda Wars Chromium DevTools" );
#endif

	GetBrowser()->GetHost()->ShowDevTools(windowInfo, m_CefClientHandler, settings);
}