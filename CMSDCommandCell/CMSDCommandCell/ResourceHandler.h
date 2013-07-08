//
//
//

#pragma once

#include <vector>
#include <string>
#include <map>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/timer.hpp>

#include "Utils.h"

//#include "agent.hpp"
//#include "config.hpp"

#include "CMSDIntegrator.h"
#include "NIST/config.h"
//#include "NIST/AgentCfg.h"
//#include "NIST/MTConnectStreamsParser.h"
//#include "NIST/MTConnectDeviceParser.h"

#include "StateModel.h"
using namespace StateModel;

#include "CmdCell.h"


//template <typename T=CMonitor>
class CResourceHandler// : public CDESMonitor
{
public:
	CCMSDIntegrator *					_cmsd;
	Resource *							_resource;
	CDESMonitor *						_statemachine;
	std::map<std::string, Stats>		_partStats;

	CResourceHandler(Resource * r, CCMSDIntegrator * cmsd);
	//CResourceHandler(AgentConfiguration * agent, AgentMgr * config, Resource * r, CCMSDIntegrator * cmsd);
	~CResourceHandler(void);
	//void Configure(crp::Config	& config, 
	//	std::string id,				// CMSD id of resource , 
	//	std::string device,			// name of the command , 
	//	std::string xmldevice,		 // type if device
	//	std::string ipaddr,			// address of the corresponding status
	//	std::string devicexmlpath); // location of devices

	//void SetMTCTagValue(std::string device, std::string tag, std::string value);

	//CJobCommands * jobs;
	CJobCommand * job;  // my current/last job
	CJobCommand * CurrentJob() { return job; }
	//bool SendCommand(ProcessPlan * plan);
	//ProcessPlan * CurrentPlan() { return _currentplan; }

	//ProcessPlan * _currentplan;
	//AgentConfiguration *				_agent; // command access to command agent through _agentconfig
	//AgentMgr *				_agentconfig; // command access to command agent through _agentconfig
	//MTConnectStreamsParser				_parser;      // access to agent status through _parser
	//MTConnectDataModel					_model;

	//std::string							_device, _xmldevice, _ipaddr,_devicexmlpath;
	bool								_mRunning;
	std::string							_execution;
	int									_nCmdnum;
	std::string							_identifier;

};

class CResourceHandlers : public std::vector<CResourceHandler * >
{
	std::map<std::string, std::vector<CResourceHandler * > > _resourceByPartStats;
	std::map<std::string, Stats> partStats;
public:
	CResourceHandlers() {  }

	CResourceHandler * CreateResourceHandler(std::string partid, Resource * r, CCMSDIntegrator * cmsd)
	{
		CResourceHandler * resourceHandler = new CResourceHandler(r, cmsd);
		push_back(resourceHandler);
		return resourceHandler;

	}
	CResourceHandler * Find(std::string name);
	std::vector<CResourceHandler * > & GetJobResources(std::string partid) { return _resourceByPartStats[partid]; }
	void UpdateResourceHandlers();
	void ApplyAllResources(Job *	job, CCMSDIntegrator *	cmsd){}
	void CreateResourcesByPart(CCMSDIntegrator * cmsd);
};

__declspec(selectany)  CResourceHandlers Factory;



class CCellHandler : public Cell
{
public:
	CCellHandler(AgentMgr * config, Cell * cell, CCMSDIntegrator * cmsd) : Cell(*cell)
	{
		_agentconfig=config;
		_cell=this;
		//(*this)=*cell;
		_cmsd=cmsd;
	}
	~CCellHandler(void)	{}
	bool IsFull(){ return inprocessplans.size() > CJobCommands::MaxQueueSize; }
	UINT QueueSize() { return inprocessplans.size(); }
	UINT QueuePlan(ProcessPlan *plan) { inprocessplans.push_back(plan); return QueueSize() ;}
	//int Update();
	bool AnyDone(){ return outprocessplans.size() > 0; }
	ProcessPlan * PopDone()
	{ 
		if(outprocessplans.size() <= 0) return NULL; 
		ProcessPlan * plan = outprocessplans.back(); 
		outprocessplans.pop_back(); 
		return plan; 
	}

	std::vector<ProcessPlan *> inprocessplans;
	std::vector<ProcessPlan *> outprocessplans;

	bool								_mRunning;
	std::vector<CResourceHandler *>		_resources;
	AgentMgr *				_agentconfig;
	Cell *								_cell;
	CCMSDIntegrator *					_cmsd;
	boost::mutex						_access;
};



