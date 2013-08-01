// MainFrm.cpp : implmentation of the CMainFrame class
//

#include "stdafx.h"
#include "resource.h"

#include "CMSDCommandCellView.h"
#include "MainFrm.h"
#include "StdStringFcn.h"
#include "CmdCell.h"
#include "HtmlTable.h"

#define ID_ZIP_PANE 404 //For zip
#define ID_UNITS_PANE 405 //For minutes vs seconds
#define ID_FINISH_PANE 406 //For minutes vs seconds

static std::string CmsdPath("C:\\Users\\michalos\\Documents\\GitHub\\CMSD\\TESTBED DEMO\\AgentCmd\\GMCasting\\");

int CMainFrame::GetActivePage()
{

	int nActivePage = m_view.GetActivePage();
	if(nActivePage < 1 )
		nActivePage=0;
	return nActivePage;
}

CMainFrame::CMainFrame()
{
	//_cmdAgentCfg.SetHttpPort(81);
	//_devicesAgentCfg.SetHttpPort(82);
	//_cmdAgentCfg.SetXmlFile("Devices1.xml");
	//_cmdAgentCfg.SetCfgFile("Agent1.cfg");

	_cmsd = new CCMSDIntegrator ;
	//_nLoopCounter=-1;
	 _bSnapshot=_bPaused=_bStopped=false;
	 _timeDivisor=1.0;
	 _bMinutes=true;
	 _bKPISnapshot=false;
	 _bZip=false;
	 _bFinish=false;

}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();

	if(_bZip !=_bLastzip  && _bZip ) // && _bZip !=_bLastzip)
		m_status.SetPaneText(ID_ZIP_PANE, _T("Zip On")); //,SBT_NOBORDERS);
	else if(_bZip !=_bLastzip )
		m_status.SetPaneText(ID_ZIP_PANE, _T("Zip Off")); //,SBT_NOBORDERS);
	
	if(_bMinutes && _bLastMinutes !=_bMinutes ) // && _bZip !=_bLastzip)
		m_status.SetPaneText(ID_UNITS_PANE, _T("Minutes")); //,SBT_NOBORDERS);
	else if( _bLastMinutes !=_bMinutes )
		m_status.SetPaneText(ID_UNITS_PANE, _T("Seconds")); //,SBT_NOBORDERS);	

	if(_bFinish && _bLastFinish !=_bFinish ) // && _bZip !=_bLastzip)
		m_status.SetPaneText(ID_FINISH_PANE, _T("Finish")); //,SBT_NOBORDERS);
	else if( _bLastFinish !=_bFinish )
		m_status.SetPaneText(ID_FINISH_PANE, _T("")); //,SBT_NOBORDERS);	

	
	_bLastFinish=_bFinish;
	_bLastMinutes=_bMinutes;
	_bLastzip=_bZip;
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	CreateSimpleToolBar();

	CreateSimpleStatusBar();
	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	UIAddToolBar(m_hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = GetMenu();
	m_view.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	m_status.SubclassWindow(m_hWndStatusBar);

	int arrPanes[] = { ID_DEFAULT_PANE, ID_ZIP_PANE,ID_UNITS_PANE,ID_FINISH_PANE};
	m_status.SetPanes(arrPanes, sizeof(arrPanes) / sizeof(int), false);

	int arrWidths[] = { 0, 50, 80,50 };
	//m_status.SetParts(sizeof(arrWidths) / sizeof(int), arrWidths); 

	//m_status.SetPaneWidth(ID_DEFAULT_PANE, 100);
	m_status.SetPaneWidth(ID_ZIP_PANE, 40);
	m_status.SetPaneWidth(ID_UNITS_PANE, 40);
	m_status.SetPaneWidth(ID_FINISH_PANE, 40);
	
	m_status.SetPaneText(ID_DEFAULT_PANE, _T("Ready")); //,SBT_NOBORDERS);
	m_status.SetPaneText(ID_ZIP_PANE, _T("Zip Off")); //,SBT_NOBORDERS);
	m_status.SetPaneText(ID_UNITS_PANE, _T("Seconds")); //,SBT_NOBORDERS);
	m_status.SetPaneText(ID_FINISH_PANE, _T("")); //,SBT_NOBORDERS);
	
	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
//	agent.stop();

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFilePrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CWtlHtmlView *	 _pView = _pages[GetActivePage()];
	_variant_t vArg;
	_pView->ExecCommand(OLECMDID_PRINT,OLECMDEXECOPT_DODEFAULT, NULL, &vArg); 

	return 0;
}
LRESULT CMainFrame::OnFilePrintPreview(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CWtlHtmlView *	 _pView = _pages[GetActivePage()];
	_variant_t vArg;
	_pView->ExecCommand(OLECMDID_PRINTPREVIEW,OLECMDEXECOPT_DODEFAULT, NULL, &vArg); 

	return 0;
}
LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static int n=0;

	static TCHAR strFilter[] = _T("CMSD Files (*.xml)\0*.xml\0");
	CFileDialog fileDlg(TRUE,
		_T("*"),
		NULL,
		OFN_HIDEREADONLY,
		strFilter);			
	if( fileDlg.DoModal ()!=IDOK )
		return 0;
	m_FileTitle=fileDlg.m_szFileTitle;

	CWtlHtmlView * pView = new CWtlHtmlView();
	pView->Create(m_view,rcDefault,
		"about:blank", 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL,
		WS_EX_CLIENTEDGE);
	m_view.AddPage(pView->m_hWnd,fileDlg.m_szFileTitle);
	_pages.push_back(pView);
	pView->Write("<BR> Hello World");

	_cmsd->ParseCMSD(fileDlg.m_szFileName);

	GLogger.AddListener( boost::bind(&CMainFrame::LogMsg, this, _1));
	return 0;
}
LRESULT CMainFrame::OnStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
//	if(m_szFileTitle.Empty())
//		return 0;
	std::vector<char *> args;
	args.push_back((char *) m_FileTitle.c_str());
	args.push_back(0);
	//agent.thread(1, (const char **) &args[0]);

	return 0;
}
LRESULT  CMainFrame::OnDisplaySnapshot(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	std::string filename = ::ExeDirectory() + "Doc.html";
	CWtlHtmlView * pView = new CWtlHtmlView();
	pView->Create(m_view,rcDefault,
		filename.c_str(), 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL,
		WS_EX_CLIENTEDGE);
	m_view.AddPage(pView->m_hWnd,"Snapshot");
	_pages.push_back(pView);
	return 0;
}
LRESULT  CMainFrame::OnDisplayKPI(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	std::string filename = ::ExeDirectory() + "JobsKpi.html"
		;
	CWtlHtmlView * pView = new CWtlHtmlView();
	pView->Create(m_view,rcDefault,
		filename.c_str(), 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL,
		WS_EX_CLIENTEDGE);
	m_view.AddPage(pView->m_hWnd,"KPI");
	_pages.push_back(pView);
	return 0;
}
LRESULT CMainFrame::OnDisplayAgent(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	pHtmlView->SetElementId( "JobStatus",  jobs->JobStatus); // htmlElapsedTable.CreateHtmlTable() + "<BR>" + htmlJobsTable.CreateHtmlTable()+"<BR>\n");
	pHtmlView->SetElementId( "Jobs", jobs->Jobs); // jobs->htmlJobsTable.CreateHtmlTable()+"<BR>\n");
	pHtmlView->SetElementId( "Device", jobs->DeviceStatus); // jobs->htmlTable.CreateHtmlTable());
	return 0;
}

void CMainFrame::MutexStep(std::string s)
{
	OutputDebugString(s.c_str());
	//_nLoopCounter--;
	//if(_nLoopCounter<0)
		cond.notify_all();  
}

LRESULT CMainFrame::OnFileRun(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_cmsd->ParseCMSD(CmsdPath + "GMCastinglFactoryTestbed.xml");
	_cmsd->MergeCMSD(CmsdPath + "GMCastinglPerfKPI.xml");
	_cmsd->MergeCMSD(CmsdPath + "GMCastingJob.xml");

	_cmsd->MergeCMSD(CmsdPath + "GMCastinglUtilities.xml");
	OnFileProcess();
	return 0;
}

LRESULT CMainFrame::OnFileProcess()
{
	
	pHtmlView = new CWtlHtmlView();

	pHtmlView->Create(m_view,rcDefault,
		"about:blank", 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL,
		WS_EX_CLIENTEDGE);

	pHtmlView->SetDocumentText(CHtmlTable::CreateHtmlDocument().c_str());
	pHtmlView->SetElementId( "Header", "<H1>Casting 1 operation</H1>\n"
		);
	pHtmlView->SetElementId( "Buttons",
		"<BUTTON id = \"Pause\"  onClick=\"location.href='pausehost'\" >PAUSE</BUTTON>"		
		"<BUTTON id = \"Resume\"  onClick=\"location.href='resumehost'\" >RESUME</BUTTON>"		
		"<BUTTON id = \"Stop\"  onClick=\"location.href='stophost'\" >STOP</BUTTON>"		
		"<BUTTON id = \"Step\"  onClick=\"location.href='localhost'\" >STEP</BUTTON>"
		"<BUTTON id = \"Finish\"  onClick=\"location.href='finishhost'\" >FINISH</BUTTON>"
		"<BUTTON id = \"Snapshot\"  onClick=\"location.href='snapshothost'\" >SNAPSHOT</BUTTON>"
		"<BUTTON id = \"KPI\"  onClick=\"location.href='KPIhost'\" >KPI</BUTTON>"
		"<BUTTON id = \"Zip\"  onClick=\"location.href='ziphost'\" >ZIP</BUTTON>"
		"<BUTTON id = \"Units\"  onClick=\"location.href='unitshost'\" >UNITS</BUTTON>"
		"<BUTTON id = \"Run\"  onClick=\"location.href='runhost'\" >RUN</BUTTON>"
		"<input type=\"text\" id=\"Loop\" size=\"6\" >"
		"<BUTTON id = \"Deadline\"  onClick=\"location.href='deadlinehost'\" >DEADLINE </BUTTON>\n"
		"<input type=\"text\" id=\"ContinueLoop\" size=\"6\">\n"
	);

	m_view.AddPage(pHtmlView->m_hWnd,"Merged CMSD");
	_pages.push_back(pHtmlView);

	pHtmlView->AddListener("Step", boost::bind(&CMainFrame::MutexStep, this,_1));

	jobs = new CJobCommands( _cmsd);
	Factory.CreateResourcesByPart(_cmsd);

	jobs->InitAllJobs((Job *)  _cmsd->jobs->at(0).get());// there may be multiple jobs - but only 1st counts in our world
	jobs->InitJobsStats((Job *)  _cmsd->jobs->at(0).get()) ;  // FIXME: verify works

	std::string nMaxQueueSize = _cmsd->jobs->at(0)->GetPropertyValue("MaxQueueSize");
	CJobCommands::MaxQueueSize=ConvertString<int>(nMaxQueueSize,2);
 	jobs->Run(jobs);

	return 0;
}

LRESULT CMainFrame::OnFileMerge(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static int n=0;

	static TCHAR strFilter[] = _T("CMSD Files (*.xml)\0*.xml\0");
	CFileDialog fileDlg(TRUE,
		_T("*"),
		NULL,
		OFN_HIDEREADONLY,
		strFilter);			
	if( fileDlg.DoModal ()!=IDOK )
		return 0;

	_cmsd->MergeCMSD(fileDlg.m_szFileName);

	// FIXME: Handle errors...

	return 0;
}

LRESULT CMainFrame::OnLogMessage(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	char * str = (char *) wParam;
	_pages.back()->Write(str);
	delete str;
	return 0;
}

void CMainFrame::LogMsg(const TCHAR * msg)
{
	std::string txt(msg);
	ReplaceAll(txt, "\n", "<BR>\n");
	char * str = new char[txt.size()+1];
	strncpy(str,txt.c_str(),txt.size()+1);
	PostMessage(LOG_MSG, (WPARAM) str, 0);

}


LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndToolBar);
	::ShowWindow(m_hWndToolBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}


LRESULT CMainFrame::OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nActivePage = m_view.GetActivePage();
	if(nActivePage < 1 )
	{
		::MessageBeep((UINT)-1);
		return 0;
	}
	_pages.erase(_pages.begin() + nActivePage);
	if(nActivePage != -1)
		m_view.RemovePage(nActivePage);


	return 0;
}

LRESULT CMainFrame::OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(_pages.size() < 1 )
	{
		::MessageBeep((UINT)-1);
		return 0;
	}
	for(int i=1; i< _pages.size(); i++)
	{
		_pages.erase(_pages.begin() + i);
		m_view.RemovePage(i);
	}
	return 0;
}

LRESULT CMainFrame::OnWindowActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nPage = wID - ID_WINDOW_TABFIRST;
	m_view.SetActivePage(nPage);
	return 0;
}

LRESULT CMainFrame::OnDisplayResource(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{	
	int i = (WPARAM) wParam;
	std::string filename = ::ExeDirectory()+Factory[i]->_statemachine->Name()+ ".html";

	CWtlHtmlView * pView = new CWtlHtmlView();
	pView->Create(m_view,rcDefault,
		filename.c_str(), 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL,
		WS_EX_CLIENTEDGE);
	m_view.AddPage(pView->m_hWnd,"Snapshot");
	_pages.push_back(pView);
	return 0;
}
LRESULT CMainFrame::OnFileBaseline(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_cmsd->ParseCMSD(CmsdPath + "GMCastinglFactoryTestbed.xml");
	_cmsd->MergeCMSD(CmsdPath + "GMCastinglPerfKPI0.xml");
	_cmsd->MergeCMSD(CmsdPath + "GMCastingJob.xml");

	_cmsd->MergeCMSD(CmsdPath + "GMCastinglUtilities.xml");
	OnFileProcess();

	return 0;
}
