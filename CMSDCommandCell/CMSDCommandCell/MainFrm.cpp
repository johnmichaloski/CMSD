// MainFrm.cpp : implmentation of the CMainFrame class
//

#include "stdafx.h"
#include "resource.h"

#include "CMSDCommandCellView.h"
#include "MainFrm.h"
#include "StdStringFcn.h"
#include "CmdCell.h"
#include "HtmlTable.h"

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
	 _bMinutes=false;
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
	_variant_t vArg;
	pHtmlView->ExecCommand(OLECMDID_PRINT,OLECMDEXECOPT_DODEFAULT, NULL, &vArg); 

	return 0;
}
LRESULT CMainFrame::OnFilePrintPreview(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_variant_t vArg;
	pHtmlView->ExecCommand(OLECMDID_PRINTPREVIEW,OLECMDEXECOPT_DODEFAULT, NULL, &vArg); 

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
//LRESULT CMainFrame::OnDisplaySnapshot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

LRESULT CMainFrame::OnDisplayAgent(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CHtmlTable htmlJobsTable,htmlElapsedTable,htmlTable; 

	//Job* job = (Job *)  _cmsd->jobs->at(0).get(); // there may be multiple jobs - but only 1st counts in our world
	std::string  elapsedHeader="Date,Time, Simulation<br>Seconds, Togo, Deadline";
	htmlElapsedTable.SetHeaderColumns( elapsedHeader);
	std::string  jobElapsed=jobs->serviceTime.GetCurrentDateTime() + "," ;
	jobElapsed += jobs->serviceTime.ElapsedString() + ",";
	jobElapsed += StdStringFormat("%d,", (int) jobs->TimeElapsed());
	jobElapsed += StdStringFormat("%d,", (int) (jobs->Deadline()-jobs->TimeElapsed()));
	jobElapsed += StdStringFormat("%d ", (int) jobs->Deadline());
	htmlElapsedTable.AddRow(elapsedHeader, jobElapsed);

	////////////////////////////////////////////////////////
	std::string	jobsheader = "Job, Finished Parts, TotalParts" ;
	htmlJobsTable.SetHeaderColumns( jobsheader);
	std::string  jobHtml;
	for(int i=0 ; i< jobs->parts.size(); i++)
	{
		jobHtml =  jobs->parts[i] +",";
		jobHtml +=StdStringFormat("%6d,%6d", jobs->finishedparts[ jobs->parts[i]],jobs->totnumparts[jobs->parts[i]]); 
		htmlJobsTable.AddRow(jobsheader, jobHtml);
	}

	pHtmlView->SetElementId( "JobStatus", htmlElapsedTable.CreateHtmlTable() + "<BR>" + htmlJobsTable.CreateHtmlTable()+"<BR>\n");
	////////////////////////////////////////////////////////
	jobsheader = "JobId,PartId, Current, Operation, Machine,Max Steps,Order Time, Factory Time" ;
	htmlJobsTable.ClearValues();
	htmlJobsTable.SetHeaderColumns( jobsheader);
	jobHtml.clear();
	for(int i=0 ; i< jobs->size(); i++)
	{
		std::string step;
		int k = jobs->at(i)->_currentstep;
		if(k>=0 && k <  jobs->at(i)->steps.size())
			step = jobs->at(i)->steps[k];
		jobHtml =  jobs->at(i)->_jobId +",";
		jobHtml +=  jobs->at(i)->_partid +",";
		
		jobHtml +=StdStringFormat("%6d,%s,", jobs->at(i)->_currentstep,step.c_str()); 
		if(k>=0 && k <  jobs->at(i)->_ResourceHandlers.size())
			jobHtml +=  jobs->at(i)->_ResourceHandlers[k]->_identifier + ",";
		else
			jobHtml += ",";
		jobHtml +=StdStringFormat("%6d", jobs->at(i)->MaxStep())+ ","; 
		jobHtml +=jobs->at(i)->orderTime.ElapsedString() + ",";
		jobHtml +=jobs->at(i)->factoryTime.ElapsedString()  ;
		htmlJobsTable.AddRow(jobsheader, jobHtml);
	}

	pHtmlView->SetElementId( "Jobs", htmlJobsTable.CreateHtmlTable()+"<BR>\n");
	////////////////////////////////////////////////////////////////
	std::string units = _bMinutes ? "Minutes" : "Seconds";
	std::string	header = "#,Machine,State,InQ,InQMax,Current,MTP,Processing Left," + _resourceHandlers[0]->_statemachine->GenerateCSVHeader(units);
	htmlTable.SetHeaderColumns( header);

	for(int i=0;i<_resourceHandlers.size() ; i++)
	{
		std::string html1=StdStringFormat("%d,",i);

		html1+= StdStringFormat("<A HREF=\"%s\">",(::ExeDirectory()+_resourceHandlers[i]->_statemachine->Name()).c_str() );
		html1+=_resourceHandlers[i]->_statemachine->Name() + "</A>,";

		//html1+=_resourceHandlers[i]->_statemachine->Name() + ",";
		html1+=StateStr(_resourceHandlers[i]->_statemachine->GetState())  + "," ;
		html1+=StdStringFormat("%d", _resourceHandlers[i]->_statemachine->size())  + "," ;
		html1+=StdStringFormat("%d", _resourceHandlers[i]->_statemachine->MaxSize())  + "," ;
		if(_resourceHandlers[i]->_statemachine->Current()!=NULL) 
			html1+="*,"; 
		else html1+="_,";  
		html1+=StdStringFormat("%8.4f", _resourceHandlers[i]->_statemachine->MTTP)  + "," ;
		if(_resourceHandlers[i]->_statemachine->Current()!=NULL)
			html1+=StdStringFormat("%8.4f", _resourceHandlers[i]->_statemachine->Current()->_mttp)  + "," ;
		else html1+=" ,";
		
		html1+= _resourceHandlers[i]->_statemachine->GenerateCSVTiming(_timeDivisor)  ;
		htmlTable.AddRow(header, html1);
	}

	pHtmlView->SetElementId( "Device", htmlTable.CreateHtmlTable());
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
	_cmsd->ParseCMSD("C:\\Program Files\\NIST\\proj\\MTConnect\\Nist\\MTConnectGadgets\\TESTBED DEMO\\AgentCmd\\GMCasting\\GMCastinglFactoryTestbed.xml");
	_cmsd->MergeCMSD("C:\\Program Files\\NIST\\proj\\MTConnect\\Nist\\MTConnectGadgets\\TESTBED DEMO\\AgentCmd\\GMCasting\\GMCastinglPerfKPI.xml");
	_cmsd->MergeCMSD("C:\\Program Files\\NIST\\proj\\MTConnect\\Nist\\MTConnectGadgets\\TESTBED DEMO\\AgentCmd\\GMCasting\\GMCastingJob.xml");

	_cmsd->MergeCMSD("C:\\Program Files\\NIST\\proj\\MTConnect\\Nist\\MTConnectGadgets\\TESTBED DEMO\\AgentCmd\\GMCasting\\GMCastinglUtilities.xml");
	
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
		"<BUTTON id = \"Snapshot\"  onClick=\"location.href='snapshothost'\" >SNAPSHOT</BUTTON>"
		"<BUTTON id = \"Seconds\"  onClick=\"location.href='secondshost'\" >SECONDS</BUTTON>"
		"<BUTTON id = \"Minutes\"  onClick=\"location.href='minuteshost'\" >MINUTES</BUTTON>"
		"<BUTTON id = \"Run\"  onClick=\"location.href='runhost'\" >RUN</BUTTON>"
		"<input type=\"text\" id=\"Loop\" size=\"6\" >"
		"<BUTTON id = \"Continue\"  onClick=\"location.href='continuehost'\" >CONTINUE </BUTTON>\n"
		"<input type=\"text\" id=\"ContinueLoop\" size=\"6\">\n"
	);

	m_view.AddPage(pHtmlView->m_hWnd,"Merged CMSD");
	_pages.push_back(pHtmlView);

	pHtmlView->AddListener("Step", boost::bind(&CMainFrame::MutexStep, this,_1));

	for(int i=0 ; i< _cmsd->resources->size(); i++)
	{
		Resource * r = (Resource *) _cmsd->resources->at(i).get();
		std::string name = (LPCSTR) r->name;
		if(name.empty() || name == "None")  // something wrong
			continue; 

		CResourceHandler *  _resourceHandler = new CResourceHandler(r, _cmsd);
		_resourceHandler->_identifier = (LPCSTR) r->identifier;
		_resourceHandler->_statemachine->name = name;
		_resourceHandler->_statemachine->SetStateMachineName(name);
		_resourceHandlers.push_back(_resourceHandler);
	}


	//jobs = new CJobCommands(&agent, _cmsd);
	jobs = new CJobCommands( _cmsd);
	jobs->InitAllJobs((Job *)  _cmsd->jobs->at(0).get());// there may be multiple jobs - but only 1st counts in our world

	std::string nMaxQueueSize = _cmsd->jobs->at(0)->GetPropertyValue("MaxQueueSize");
	CJobCommands::MaxQueueSize=ConvertString<int>(nMaxQueueSize,2);
 	jobs->Run(jobs,&_resourceHandlers);

	return 0;
}



LRESULT CMainFrame::UpdateResourceHandlers()
{	
	for(int i=0 ; i<_resourceHandlers.size(); i++)
	{
		CResourceHandler *  _resourceHandler = _resourceHandlers[i];
		_resourceHandler->_statemachine->Cycle();
	}
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

