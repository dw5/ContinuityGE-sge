/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "ShaderTestApp.h"

#include "MainFrm.h"
#include "ShaderTestDoc.h"
#include "ShaderTestView.h"

#include "engine/cameraapi.h"
#include "engine/engineapi.h"
#include "engine/entityapi.h"
#include "engine/saveloadapi.h"
#include "platform/inputapi.h"
#include "render/renderapi.h"
#include "tech/globalobj.h"
#include "tech/imageapi.h"
#include "tech/resourceapi.h"
#include "tech/schedulerapi.h"
#include "tech/techtime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace ShaderTest
{
   cFrameLoopClient::~cFrameLoopClient()
   {
   }
}

/////////////////////////////////////////////////////////////////////////////
// cShaderTestApp

BEGIN_MESSAGE_MAP(cShaderTestApp, CWinApp)
	//{{AFX_MSG_MAP(cShaderTestApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cShaderTestApp construction

cShaderTestApp::cShaderTestApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////

BOOL cShaderTestApp::AddLoopClient(ShaderTest::cFrameLoopClient * pLoopClient)
{
   if (pLoopClient != NULL)
   {
      m_loopClients.Add(pLoopClient);
      return TRUE;
   }
   return FALSE;
}

BOOL cShaderTestApp::RemoveLoopClient(ShaderTest::cFrameLoopClient * pLoopClient)
{
   if (pLoopClient != NULL)
   {
      for (int i = 0; i < m_loopClients.GetSize(); i++)
      {
         if (m_loopClients[i] == pLoopClient)
         {
            m_loopClients.RemoveAt(i);
            return TRUE;
         }
      }
   }
   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only cShaderTestApp object

cShaderTestApp theApp;

/////////////////////////////////////////////////////////////////////////////
// cShaderTestApp initialization

static void RegisterGlobalObjects()
{
   CameraControlCreate();
   InputCreate();
   SaveLoadManagerCreate();
   SchedulerCreate();
   ResourceManagerCreate();
   EntityManagerCreate();
   RendererCreate();
}

BOOL cShaderTestApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MFC_VER < 0x0700
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      ErrorMsg("One or more application-level services failed to start!\n");
      return FALSE;
   }

   TextFormatRegister(NULL);
   EngineRegisterResourceFormats();
   ImageRegisterResourceFormats();

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(cShaderTestDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(cShaderTestView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

int cShaderTestApp::ExitInstance() 
{
	StopGlobalObjects();
	
	return CWinApp::ExitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void cShaderTestApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// cShaderTestApp message handlers


int cShaderTestApp::Run()
{
   ASSERT_VALID(this);

   // for tracking the idle time state
   bool bIdle = true;
   long lIdleCount = 0;

   static const double kFrameDelay = 0.1;

   double timeLastFrame = TimeGetSecs();
   double time = timeLastFrame + (2 * kFrameDelay);

   UseGlobal(Scheduler);
   pScheduler->Start();

   MSG lastMouseMove = {0};

   // acquire and dispatch messages until a WM_QUIT message is received.
   for (;;)
   {
#if _MFC_VER >= 0x0700
	   _AFX_THREAD_STATE * pState = AfxGetThreadState();
      MSG * pMsg = &(pState->m_msgCur);
#else
      MSG * pMsg = &m_msgCur;
#endif

      // phase1: check to see if we can do idle work
      while (bIdle &&
         !::PeekMessage(pMsg, NULL, 0, 0, PM_NOREMOVE))
      {
         // call OnIdle while in bIdle state
         if (!OnIdle(lIdleCount++))
            bIdle = false; // assume "no idle" state
      }

      // phase2: pump messages while available
      while (::PeekMessage(pMsg, NULL, 0, 0, PM_NOREMOVE))
      {
         // pump message, but quit on WM_QUIT
         if (!PumpMessage())
            return ExitInstance();

         // reset "no idle" state after pumping "normal" message
         if (IsIdleMessage(pMsg))
         {
            bIdle = true;
            lIdleCount = 0;
         }
      }

      double elapsed = time - timeLastFrame;

      if (elapsed > kFrameDelay)
      {
         pScheduler->NextFrame();

         for (int i = 0; i < m_loopClients.GetSize(); i++)
         {
            m_loopClients[i]->OnFrame(time, elapsed);
         }

         timeLastFrame = time;
      }

      time = TimeGetSecs();
   }

   Assert(!"Should never reach this point!"); // not reachable
}
