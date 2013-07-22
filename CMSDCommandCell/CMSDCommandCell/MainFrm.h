// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
//#include "AgentThread.h"
//#include "NIST/AgentCfg.h"
//#include "MTCAgentCmd.h"
#include "Resource.h"
#include "WtlHtmlView.h"
#include "CMSDIntegrator.h"
#include "HtmlTable.h"
#include <boost/thread.hpp>
#include "CmdCell.h"
#include "ResourceHandler.h"

#define WINDOW_MENU_POSITION	3
#define SIMULATION_RUN_DONE (WM_APP  + 1)
#define LOG_MSG (WM_APP  + 2)
#define DISPLAY_MSG (WM_APP  + 3)
#define DISPLAY_SNAPSHOT (WM_APP  + 4)
#define DISPLAY_KPISNAPSHOT (WM_APP  + 5)
#define DISPLAY_RESOURCE (WM_APP  + 6)



class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
		public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CMainFrame();

	boost::mutex _access;
	boost::condition_variable cond;
	CJobCommands * jobs;
	bool _bPaused,_bStopped,_bSnapshot,_bKPISnapshot;

	CTabView m_view;
	std::vector<CWtlHtmlView *> _pages;
	CWtlHtmlView *	pHtmlView ;
	CMultiPaneStatusBarCtrl	 m_status;

	//int _nLoopCounter;
	void LogMsg(const TCHAR * msg);
	void MutexStep(std::string s);


	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_WINDOW_CLOSE, OnWindowClose)
		COMMAND_ID_HANDLER(ID_WINDOW_CLOSE_ALL, OnWindowCloseAll)
		COMMAND_ID_HANDLER(ID_FILE_PRINT, OnFilePrint)
		COMMAND_ID_HANDLER(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
		MESSAGE_HANDLER(DISPLAY_SNAPSHOT, OnDisplaySnapshot)
		MESSAGE_HANDLER(DISPLAY_KPISNAPSHOT, OnDisplayKPI)
		MESSAGE_HANDLER(DISPLAY_RESOURCE, OnDisplayResource)
		
		COMMAND_ID_HANDLER(DISPLAY_MSG, OnDisplayAgent)
		MESSAGE_HANDLER(LOG_MSG, OnLogMessage)
		//MESSAGE_HANDLER(DISPLAY_SNAPSHOT, OnDisplaySnapshot)
		
		COMMAND_ID_HANDLER(ID_FILE_START, OnStart)
		COMMAND_ID_HANDLER(ID_FILE_MERGE, OnFileMerge)
		COMMAND_ID_HANDLER(ID_FILE_RUN, OnFileRun)
		
		COMMAND_RANGE_HANDLER(ID_WINDOW_TABFIRST, ID_WINDOW_TABLAST, OnWindowActivate)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnFilePrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFilePrintPreview(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
LRESULT OnWindowActivate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLogMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	LRESULT OnFileMerge(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileRun(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDisplayAgent(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDisplaySnapshot(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDisplayKPI(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDisplayResource(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	std::string										 m_FileTitle;
	CCMSDIntegrator *								_cmsd;
	//std::string										header;
	double											_timeDivisor;
	bool											_bMinutes,_bLastMinutes;
	bool                                            _bZip, _bLastzip;
	bool											_bFinish,_bLastFinish;

};
