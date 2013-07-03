//
// AgentThread.h
//

// This software was developed by U.S. Government employees as part of
// their official duties and is not subject to copyright. No warranty implied 
// or intended.

#pragma once
#include <boost/thread.hpp>

class CCMSDCommandCellView;
class CAgentThread
{
public:
	CAgentThread(CCMSDCommandCellView *);
	~CAgentThread(void);
	boost::thread					_agentthread;
	void							Run();
	//void							StopHeartbeat() { _agentthread.interrupt(); }
	void							Start() {_agentthread= boost::thread(boost::bind(&CAgentThread::Run, this)); }
	CCMSDCommandCellView * _view;
};
