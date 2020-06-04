// RenderSoft CamStudio
//
// Copyright 2001 RenderSoft Software & Web Publishing

#include "stdafx.h"
#include "Recorder.h"

#include "MainFrm.h"
#include "RecorderDoc.h"
#include "RecorderView.h"
#include "AboutDlg.h"

#include "buildinfo.h"
#include "gdi_plus_initializer.h"
#include "utility/string_convert.h"
#include "utility/filesystem.h"
#include <logging/logging.h>
#include <CamLib/CamError.h>
#include <filesystem>

static auto logger = logging::get_logger("recorder");

static bool bClassRegistered = false;

/////////////////////////////////////////////////////////////////////////////
// CRecorderApp
constexpr auto CAMSTUDIO_MUTEX = _T("VSCAP_CAB648E2_684F_4FF1_B574_9714ACAC6D57");
constexpr auto CAMSTUDIO_CLASS_NAME = L"CamStudio";

BEGIN_MESSAGE_MAP(CRecorderApp, CWinApp)
ON_COMMAND(ID_APP_ABOUT, &CRecorderApp::OnAppAbout)
ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

CRecorderApp theApp;

CRecorderApp::CRecorderApp()
    : CWinApp()
    , m_hAppMutex(INVALID_HANDLE_VALUE)
{
}

CRecorderApp::~CRecorderApp() = default;

BOOL CRecorderApp::InitInstance()
{
    const auto app_data_path = utility::get_app_data_path();
    std::filesystem::create_directories(app_data_path);

    logger->info("CamStudio started, version: {}, builddate: {}", buildinfo::full_version,
        buildinfo::build_date);

    CWinApp::InitInstance();
    AfxInitRichEdit2();

    // Initialize GDI+.
    m_gdi = std::make_unique<gdi_plus>();

    logger->info("CRecorderApp::InitInstance");
    AfxEnableControlContainer();

    if (!FirstInstance())
        return FALSE;

    // new class and exit if it fails
    if (!RegisterWindowClass())
    {
        logger->error("CRecorderApp::InitInstance, Class Registration Failed");
        return FALSE;
    }

#if (WINVER < 0x600) // Windows Vista
//#ifdef _AFXDLL
//    Enable3dControls();            // Call this when using MFC in a shared DLL
//#else
//    Enable3dControlsStatic();    // Call this when linking to MFC statically
//#endif
#endif // Windows Vista

    // the application's document templates. Document templates
    // serve as the connection between documents, frame windows and views.
    AddDocTemplate(new CSingleDocTemplate(IDR_MAINFRAME,
        RUNTIME_CLASS(CRecorderDoc),
        RUNTIME_CLASS(CMainFrame), // main SDI frame window
        RUNTIME_CLASS(CRecorderView)));

    // dummy commandline parser.
    CCommandLineInfo cmd_info;
    ParseCommandLine(cmd_info);
    if (!ProcessShellCommand(cmd_info))
        return FALSE;

    // The one and only window has been initialized, so show and update it.
    m_pMainWnd->ShowWindow(SW_SHOW);
    m_pMainWnd->UpdateWindow();

    return TRUE;
}

int CRecorderApp::ExitInstance()
{
    spdlog::shutdown();

    if (bClassRegistered)
        ::UnregisterClass(CAMSTUDIO_CLASS_NAME, AfxGetInstanceHandle());

    m_gdi.reset();
    return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CRecorderApp commands

// App command to run the dialog
void CRecorderApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

// Determine if another window with our class name exists...
// return TRUE if no previous instance running;    FALSE otherwise
BOOL CRecorderApp::FirstInstance()
{
    m_hAppMutex = ::CreateMutex(nullptr, TRUE, CAMSTUDIO_MUTEX);
    if (m_hAppMutex == nullptr)
    {
        logger->error("CRecorderApp::FirstInstance, unable to create named mutex");
        return false; // unable to check mutex, assume previous instance and quit.
    }
    // check last error to see if mutex existed
    if (::GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // previous instance exists
        logger->info("CRecorderApp::FirstInstance, previous instance exists");
        // todo: activate running instance
        return false;
    }

    // check older version class name
    CWnd *pWndPrev = CWnd::FindWindow(CAMSTUDIO_CLASS_NAME, nullptr);
    if (pWndPrev != nullptr)
    {
        // If iconic, restore the main window
        if (pWndPrev->IsIconic())
        {
            pWndPrev->ShowWindow(SW_RESTORE);
        }

        // Bring the main window or its popup to the foreground
        CWnd *pWndChild = pWndPrev->GetLastActivePopup();
        pWndChild->SetForegroundWindow(); // and we are done activating the previous one.
    }
    TRACE("FirstInstance: %s\n", (pWndPrev != nullptr) ? "false" : "true");
    return pWndPrev == nullptr;
}

// unique application class name that we wish to use
bool CRecorderApp::RegisterWindowClass()
{
    // todo: add mutex code
    WNDCLASS wndcls;
    memset(&wndcls, 0, sizeof(WNDCLASS)); // start with nullptr defaults
    wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = ::DefWindowProc;
    wndcls.hInstance = AfxGetInstanceHandle();
    wndcls.hIcon = LoadIcon(IDR_MAINFRAME);
    wndcls.hCursor = LoadCursor(IDC_ARROW);
    wndcls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndcls.lpszMenuName = nullptr;

    // Specify our own class name for using FindWindow later
    wndcls.lpszClassName = CAMSTUDIO_CLASS_NAME;

    // new class and exit if it fails
    if (!AfxRegisterClass(&wndcls))
    {
        TRACE("Class Registration Failed\n");
        return false;
    }
    bClassRegistered = true;
    return true;
}
