//
// CmdCell.h
//
// This software was developed by U.S. Government employees as part of
// their official duties and is not subject to copyright. No warranty implied 
// or intended.
// C:\Program Files\NIST\proj\MTConnect\Nist\MTConnectGadgets\CMSDCommandCell\CMSDCommandCell\CmdCell.h

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

class AgentMgr;
class CResourceHandler;
class CPartSequence;
class CJobSequencer;
class CCommand;
class CCellHandler;
//class CJobHandler;
//class CPartHandler;
//class CProcessPlanHandler;
class CJobCommand;
class CJobCommands;
class CResourceHandlers;

template <typename T= CJobCommand>
class Queue : public std::vector<T *>
{
	int nMaxSize;
public:
	bool Push(T * job) { if(size() < nMaxSize) { push_back(job); return true; } else return false; }
	T * Pop() { T * job =NULL; if(size() >0) {job=front(); erase(begin());}  return job; }
	bool IsWaiting() { return ((size()-1) >0 )  && ((nMaxSize-1) > 0); }
	int & MaxSize() { return nMaxSize; }
	bool CanPush() 
	{ 
		if(size() < nMaxSize) 
			return true;  
		else
			return false; 
	}
	bool CanPop()
	{
		if(size() > 0 ) 
			return true;  
		else
			return false; 
	}
	T* Current() 
	{ 
		if(CanPop()) 
			return front(); 
		else 
			return NULL; 
	}
};


// Sim StateModel - off, idle, starved, blocked, running, faulted

class CDESMonitor : public ControlThread, public Queue<>
{
public:
	double MTBF, _mtbf; // counter seconds
	double MTTR, _mttr; // counter seconds
	double MTTP;  // counter seconds

	typedef CDESMonitor MyClass;

	CDESMonitor();
	//ControlThread & StateMachine() { return _thread; }
	std::string name;

	// State transitions
	virtual void Init() {LogMessage(name); LogMessage("Init\n"); }
	virtual void Load() { LogMessage(name); LogMessage("Load\n"); }
	virtual void Run() { LogMessage(name); LogMessage("Run\n"); }
	virtual void Stop() { LogMessage(name); LogMessage("Stop\n"); }
	virtual void Fail() {LogMessage(name);  LogMessage("Fail\n"); }
	virtual void Hold() { LogMessage(name); LogMessage("Hold\n"); }
	virtual void Reset() {LogMessage(name);  LogMessage("Reset\n"); }
	virtual void Done() {LogMessage(name);  LogMessage("Done\n"); }
	virtual void Resume() {LogMessage(name);  LogMessage("Resume\n"); }

	// State methods
	virtual void Nop()	{ LogMessage(name);LogMessage("Nop\n");	}
	virtual void Off();//	{ LogMessage("Off\n");	}
	virtual void Ready() { LogMessage(name);LogMessage("Ready\n");	 }
	virtual void Loaded() { LogMessage(name); LogMessage("Loaded\n");	}
	virtual void Running();// { LogMessage(name); LogMessage("Running\n");	 }
	virtual void Stopped() { LogMessage(name); LogMessage("Stopped\n");	 }
	virtual void Interrupted() { LogMessage(name); LogMessage("Interrupted\n");	 }
	virtual void Faulted(); //  { LogMessage("Faulted\n");	 }
	virtual void Exit() { LogMessage(name); LogMessage("Exit\n");	 }

	virtual void Blocked();// {LogMessage(name);  LogMessage("Blocked\n");	 }
	virtual void Starved();//{ LogMessage(name); LogMessage("Starved\n");	 }
	
	virtual void Postprocess();
	virtual void Preprocess();
	std::string GenerateCosts(std::string prepend="", std::string postpend="", std::string state="any");
	std::string GenerateTotalCosts(std::string prepend="", std::string postpend="");
	//double GetUpdateFactor();

	void GenerateStateReport(std::map<std::string,double> &states, double dDivisor=1000);
	

	//Queue<> inqueue;
	//Queue<> outqueue;
	//CJobCommand *  Current() { return inqueue.Current(); }
	CJobCommand *  pLastCurrent;

	void SetMTBF(double sec){ MTBF= sec ;}
	void SetMTTR(double sec){ MTTR= sec;}
	void SetMTTP(double sec){  MTTP= sec;}
	void Setup();
	virtual std::string ToString();

	std::string GenerateReport();
	std::string GenerateCSVTiming(double divider = 1.0);
	std::string GenerateCSVHeader(std::string units="Seconds");
	double nBlockedTime;
	double nStarvedTime;
	double nDownTime;
	double nProductionTime;
	double nOffTime;
	double nRepairTime;
	double nIdleTime;
	double _dUpdateRateSec;
};


class CJobCommand
{
	// job/part/process plan/process (per machine)
public:

	CJobCommand(CJobCommands * jobs);

	Job * AddJob(CCMSDIntegrator * _cmsd, int &job, std::string partid);
	std::string ToString();
	int IsDone() ;
	std::string CurrentProgram() {return currentprogram[_currentstep];}
	std::string CurrentJobId(){ return _jobId ; }
	std::string CurrentPartId(){ return _partid ;}
	int MaxStep() { return this->plan->steps.size() ; }
	//std::string Steps() { std::string tmp; for(int i=0; i< step.size(); i++) { tmp+=(LPCSTR) steps[i];  if(i>0) tmp+=","; } return tmp;}
	void LoadResources(CResourceHandlers *resources);

	std::vector<CResourceHandler * > _ResourceHandlers;
	std::vector<CTimestamp> _TimePerStep;
	

	// Back pointers
	Part* part ;
	ProcessPlan * plan;
	Job * ajob;
	//AgentMgr * config;
	CCMSDIntegrator * _cmsd;
	std::string _partid,_jobId;
	CJobCommands * _parent;
	double _mttp;  // this job has spent # time at resource, counter in seconds

	std::vector <ProcessPlan *> currentprocessplans;
	std::vector <Process *> processes;
	std::vector <CCellHandler *> currentcell;
	std::vector <std::string> currentprogram;
	std::vector<bstr_t> steps ;
	int _currentstep;
	std::string sequence, oplist,cells,operations;
	std::vector<bool> bDone;

	// These belong here - since jobs lumps all info
	std::vector<std::string> _ProcessPlanId;
	std::vector<std::string> _CellResourceRequired;
	std::vector<std::string> _ResourceRequired;
	CTimestamp orderTime, factoryTime;
	
};


class CJobCommands :	public std::vector<CJobCommand * > 
{
public:
//	CJobCommands(AgentMgr * _agentconfig, CCMSDIntegrator * _cmsd);
	CJobCommands(CCMSDIntegrator * _cmsd);

	void IncPart(std::string partid);
	std::string GetPartId(int i) { return random_part[i]; }
	void InitAllJobs(Job *	masterjob) ;
	void Update(CResourceHandlers& _resourceHandlers);
	CJobCommand * AddJob(CCMSDIntegrator * _cmsd, int &jobId, std::string partid);

	int DoneParts(std::string partid) { return finishedparts[ partid]; }
	int PartsInProcess();
	int AllFinished() ;
	int IsNewWorkorder() { return this->size() < MaxQueueSize; }
	int GetCurrentNuber() { return this->size();}
	int MaxSize() { return MaxQueueSize;}

	void Update(CResourceHandlers *);
	int FindResourceHandler(std::vector<CResourceHandler * >&, int current,std::string id );
	LRESULT UpdateResourceHandlers(std::vector<CResourceHandler * >&  _resourceHandlers);
	//void DumpJobs(CJobCommands * jobs,std::vector<CResourceHandler * >&  _resourceHandlers);
	void process(CJobCommands * jobs, CResourceHandlers *  resourceHandlers);
	void Run(CJobCommands * jobs, CResourceHandlers*  _resourceHandlers);

	void Newworkorder();
	double &TimeElapsed() { return _dUpdateRateSec; }
	double &Deadline() { return _dDeadline; }
	std::string GenerateHtmlReport();

	static int MaxQueueSize;
	std::vector< std::string > parts;
	std::map<std::string, int > numparts, finishedparts,totnumparts;
	std::vector<CJobCommand  * > archive;
	std::vector<std::string> random_part;
	
	//std::vector<bstr_t> jobids ;
	std::vector<std::string> partids ;

	int _jobid;
	CCMSDIntegrator * cmsd;
	CTimestamp serviceTime;

private:
	 boost::thread    m_Thread;  
	bool m_bRunning;
	double _dUpdateRateSec;
	double _dDeadline;
	CResourceHandlers *  _resourceHandlers;
};

//template <typename T=CMonitor>
class CResourceHandler// : public CDESMonitor
{
public:
	CCMSDIntegrator *					_cmsd;
	Resource *							_resource;
	CDESMonitor *						_statemachine;

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

	//crp::Config							_config;
	//std::string							_device, _xmldevice, _ipaddr,_devicexmlpath;
	//std::string							_currentprogram;
	bool								_mRunning;
	std::string							_execution;
	int									_nCmdnum;


//	boost::mutex						io_mutex;
//	boost::mutex						_access;
	std::string							_identifier;

};


class CResourceHandlers : public std::vector<CResourceHandler * >
{
public:
	CResourceHandlers() { clear(); }
	CResourceHandler * Find(std::string name)
	{
		for(int i=0 ; i<size(); i++)
			if( std::string((LPCSTR) this->at(i)->_resource->name) == name) 
				return at(i); 
		return NULL; 
	}
	CResourceHandler * FindResourceHandler(int i){ return at(i) ; }
	void UpdateResourceHandlers()
	{	
		for(int i=0 ; i<size(); i++)
		{
			CResourceHandler *  _resourceHandler = at(i);
			if( _resourceHandler == NULL)
				continue;
			_resourceHandler->_statemachine->Cycle();
		}
	}
};

//class CJobHandler
//{
//public:
//	CJobHandler(AgentMgr * config, 
//		Job * job, 
//		CCMSDIntegrator * cmsd,
//		std::vector<CCellHandler * >	& cellHandlers)
//		: _cellHandlers(cellHandlers)
//	{
//		_agentconfig=config;
//		_job=job;
//		_cmsd=cmsd;
//		_cellHandlers=cellHandlers;
//	}
//	~CJobHandler(void){}
//
//	CJobHandler(std::vector<CCellHandler * >	& cellHandlers) : _cellHandlers(cellHandlers)
//	{
//	
//	}
//	void Init(Job * job);
//	std::vector<CCellHandler * >	&				_cellHandlers;
//	//std::vector<CJobCommand * > jobs;
//	CJobCommands jobs;
//
//	void Cycle();
//	void GenerateReport();
//	void CheckNewworkorder();
//	void CheckNewJob();
//
//	std::vector<ProcessPlan *> currentprocessplans;
//	int GetPartIndex(TCHAR * id)
//	{
//		for(int i=0; i< _job->partIds.size(); i++) 
//			if( _job->partIds[i] == _bstr_t(id)) 
//				return i;
//		return -1;
//	}
//
//
//	bool								_mRunning;
//	AgentMgr *				_agentconfig;
//	Job *								_job;
//	CCMSDIntegrator *					_cmsd;
//	boost::mutex						_access;
//
//	std::vector< std::string > parts;
//	std::vector< int > totnumparts;
//	std::vector< int > numparts, finishedparts;
//	std::vector<ProcessPlan*> _doneprocessplans;
//	std::vector<ProcessPlan*> _activeprocessplans;
//
//};
//class CPartHandler
//{
//public:
//	CPartHandler::CPartHandler(AgentMgr * config, Part * part, CCMSDIntegrator * cmsd)
//	{
//		_agentconfig=config;
//		_part=part;
//		_cmsd=cmsd;
//	}
//	~CPartHandler(void){}
//	void Cycle(){}
//
//	bool								_mRunning;
//	AgentMgr *							_agentconfig;
//	Part *								_part;
//	CCMSDIntegrator *					_cmsd;
//
//
//};


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