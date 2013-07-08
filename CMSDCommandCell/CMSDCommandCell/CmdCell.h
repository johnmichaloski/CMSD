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
class CCommand;
class CCellHandler;
class CJobCommand;
class CJobCommands;
class CResourceHandler;
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

	std::map<std::string, Stats> _partStats;
	std::string _partid;

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

	double _dUpdateRateSec;
	std::string GenerateReport();
	std::string GenerateCSVTiming(double divider = 1.0);
	std::string GenerateCSVHeader(std::string units="Seconds");
	Stats stats;
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

	void LoadResources(std::string partid);

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
	std::vector<Stats> statcoll;
	Stats & GetStepStat() { return statcoll[_currentstep]; }

	// These belong here - since jobs lumps all info
	std::vector<std::string> _ProcessPlanId;
	std::vector<std::string> _CellResourceRequired;
	std::vector<std::string> _ResourceRequired;
	CTimestamp orderTime, factoryTime;
	Stats stats;
};


class CJobCommands :	public std::vector<CJobCommand * > 
{
public:
//	CJobCommands(AgentMgr * _agentconfig, CCMSDIntegrator * _cmsd);
	CJobCommands(CCMSDIntegrator * _cmsd);

	void IncPart(std::string partid);
	std::string GetPartId(int i) { return random_part[i]; }
	void InitAllJobs(Job *	masterjob) ;
	void InitJobsStats(Job *	job) ;

	void Update(); // CResourceHandlers& _resourceHandlers);
	CJobCommand * AddJob(CCMSDIntegrator * _cmsd, int &jobId, std::string partid);

	int DoneParts(std::string partid) { return finishedparts[ partid]; }
	int PartsInProcess();
	int AllFinished() ;
	int IsNewWorkorder() { return this->size() < MaxQueueSize; }
	int GetCurrentNuber() { return this->size();}
	int MaxSize() { return MaxQueueSize;}

	int FindResourceHandler(std::vector<CResourceHandler * >&, int current,std::string id );
	void process(CJobCommands * jobs);
	void Run(CJobCommands * jobs);

	void Newworkorder();
	double &TimeElapsed() { return _dUpdateRateSec; }
	double &Deadline() { return _dDeadline; }
	std::string JobStatus; std::string Jobs; std::string  DeviceStatus;

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
	Stats stats;
	double _dUpdateRateSec;
	CTimestamp orderTime;
private:
	boost::thread    m_Thread;  
	bool m_bRunning;
	double _dDeadline;
};

