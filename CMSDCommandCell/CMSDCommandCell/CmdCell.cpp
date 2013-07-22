//
// CmdCell.cpp
//
// This software was developed by U.S. Government employees as part of
// their official duties and is not subject to copyright. No warranty implied 
// or intended.

#include "StdAfx.h"
#include "CmdCell.h"


#include "NIST/Config.h"
#include "NIST/StdStringFcn.h"
//#include "NIST/CLogger.h"
#include "NIST/Logger.h"
#include "NIST/AgentCfg.h"
//#include "MTCAgentCmd.h"
#include "MainFrm.h"
#include "HtmlTable.h"
#include "KPI.h"
#include "Reporting.h"


#include "EquationSolver.h"
using namespace EquationHelper;
using std::size_t;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::ios;
typedef EquationSolver ES;

extern CLogger FurLogger;
extern CMainFrame * _wndMain;
int CJobCommands::MaxQueueSize=2;

template<typename T>
T DOCHECK(T x, std::string y) 
{
	if (x!=NULL) 
		return x ;
	
	throw std::exception(y.c_str());
	return NULL;
}


#define IDT_TIMER 0x4B8
static void DoEvents()
{
	bool bRunning=true;
	SetTimer(NULL,IDT_TIMER, 50,(TIMERPROC) NULL);
    MSG msg;
    while (bRunning)
	{
        //if (PeekMessage(&msg, _wndMain->m_hWnd, 0, 0, PM_REMOVE))
        if (GetMessage(&msg, NULL, 0, 0))
		{
			if(msg.message == WM_TIMER)
			{
				KillTimer(_wndMain->m_hWnd, IDT_TIMER);
				bRunning=false;
					// DebugBreak();
			}

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		else  bRunning=false;
	}
		::Sleep(10); 
}



static void trans_func( unsigned int u, EXCEPTION_POINTERS* pExp )
{
	std::string errmsg =  StdStringFormat("COpcAdapter In trans_func - Code = 0x%x\n",  pExp->ExceptionRecord->ExceptionCode);
	OutputDebugString(errmsg.c_str() );
	throw std::exception(errmsg.c_str() , pExp->ExceptionRecord->ExceptionCode);
} 
static std::string GetTimeEstimate(std::string etime)
{
	if(etime.find("TRIA") != std::string::npos)
	{
		std::vector<std::string> decimals = TrimmedTokenize(etime, ",");
		int sec = ConvertString<int>(decimals[decimals.size()-1],0) * 60;
		if( decimals.size() > 0) 
			return HrMinSecFormat(sec);
	}
	return "00:00:00";

}
//////////////////////////////////////////////////////////////////////////////////////////////
CJobCommand::CJobCommand(CJobCommands * jobs)
{
	_parent=jobs; 	
	orderTime.SimStart(jobs->serviceTime.SimElapsed());
}

std::string CJobCommand::ToString()
{
	std::stringstream str;
	str<<	"Parent         " << plan->_pParentPart->identifier << std::endl;
	str<<	"Current Step   " << plan->currentStep << std::endl;
	str<<	"Current JobId  " << plan->currentJobId << std::endl;
	str<<	"Active         " << plan->bActive << std::endl;
	//	for(int i=0; i < currentcell.size(); i++);
	for(int i=0; i < processes.size(); i++)
	{
		ProcessPlan * plan=	currentprocessplans.back();

		//for(int i=0;i< 	plan->steps.size(); i++)
		//{
		//	Process *process = plan->processes[i];
		str<<	"     ==================================" << std::endl;
		str<<	"     ProcessPlan      " << i <<  "  " <<  plan->identifier  << std::endl;
		str<<	"     Process         " << i <<  "  " <<  processes[i]->identifier  << std::endl;
		str<<	"     JobId           " << i <<  "  " <<  _jobId  << std::endl;
		str<<	"     PartID          " << i <<  "  " <<  _partid  << std::endl;
		str<<	"     Cell            " << i <<  "  " <<  processes[i]->resourcesRequired[0]  << std::endl;
		Cell * cell= _cmsd->FindCellById((LPCSTR)  processes[i]->resourcesRequired[0]);
		//str<<	"     Resource        " << i <<  "  " <<  currentcell[i]  << std::endl;
		if(cell!= NULL)
			str<<	"     Resource        " << i <<  "  " <<  cell->resourceIds[0] << std::endl;

		
	}
	return str.str();
}

int CJobCommand::IsDone() 
{ 
	if(plan ==NULL) 
	{
		OutputDebugString(StdStringFormat("CJobCommand::NextStep Error no process plan \n").c_str());
		DebugBreak();
		return -1;

	}		
	int step = plan->currentStep;
	return (step >= plan->steps.size()); 
}

Job *  CJobCommand::AddJob(CCMSDIntegrator * cmsd, int &job, std::string partid)
{

	_partid=partid;
	_jobId = ConvertToString(job++);
	_cmsd=cmsd;

	ajob= new Job();
	ajob->identifier = (LPCSTR)_jobId.c_str();
	ajob->partIds.push_back((LPCSTR) _partid.c_str());
	ajob->partQuantity.push_back("1");  // only ake 1 part at time for now
	//asset.jobs->push_back(IObjectPtr((IObject *) ajob));

	Part* part = _cmsd->FindPartById( partid.c_str());
	if(part==NULL)
	{
		OutputDebugString(StdStringFormat("CJobCommand::AddPlan Error no part \n").c_str());
		DebugBreak();
		return ajob;
	}
	plan = new ProcessPlan();
	*plan = *_cmsd->FindProcessPlanById(part->processplanidentifier);

	plan->_pParentPart=part;
	plan->currentStep =0;
	plan->currentJobId =job;
	plan->jobId= (LPCSTR) _partid.c_str();
	plan->bActive=false;
	_currentstep=0;
	steps = plan->steps;;

	currentprocessplans.push_back(plan);

	//int step = plan->currentStep;

	sequence.clear(); oplist.clear();cells.clear(); operations.clear();
	for(int i=0;i< 	plan->steps.size(); i++)
	{
		Process *process = plan->processes[i];
		processes.push_back(process);
		bDone.push_back(false);
		statcoll.push_back(Stats());
		// NOT WORKING
			//Process* process = plan->FindProcess(steps[step]);
		//CCellHandler * nextcell = _cmsd->FindCellById((LPCSTR) process->resourcesRequired[0]);
		//currentcell.push_back(nextcell);


		std::string program= (LPCSTR)  plan->PartProgram(i); // or i+1 plan->currentStep);;
		currentprogram.push_back(program);

		if(i>0) sequence+=","; sequence+=StdStringFormat("%d", i*10); 
		if(i>0) oplist+=","; oplist+=plan->steps[i]; 
		if(i>0&& process!=NULL) cells+=","; cells+=process->resourcesRequired[0];
		if(i>0&& process!=NULL) operations+=","; operations+=(LPCSTR) process->description;

	}

	// Each processplan contains cell to do work, cell can have one or more resources per cell
	for(int i=0; i < processes.size(); i++)
	{
		_ProcessPlanId.push_back(  (LPCSTR) plan->identifier );
		
		Process * process =  plan->processes[i];
		_CellResourceRequired.push_back((LPCSTR)  process->resourcesRequired[0]);
		Cell * cell= _cmsd->FindCellById((LPCSTR)  process->resourcesRequired[0]);
		//str<<	"     Resource        " << i <<  "  " <<  currentcell[i]  << std::endl;
		if(cell!= NULL)
		{
			//_ResourceRequired.push_back((LPCSTR) cell->resourceIds[0]);
			Resource * r = cmsd->FindResourceById((LPCSTR) cell->resourceIds[0]);
			_ResourceRequired.push_back((LPCSTR) r->name);
		}


	}
	_currentstep=-1;  // signal not started
	return ajob;
}

void CJobCommand::LoadResources(std::string partid)
{
	std::vector<CResourceHandler * > & resources = Factory.GetJobResources(partid) ;
	_ResourceHandlers= resources; 
	_TimePerStep= std::vector<CTimestamp>(resources.size()); 

}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CJobCommands::CJobCommands(CCMSDIntegrator * _cmsd) 
{
	cmsd= _cmsd;
	_dDeadline=3000;
	_dUpdateRateSec=0;
}

CJobCommand *  CJobCommands::AddJob(CCMSDIntegrator * _cmsd, int &jobId, std::string partid)
{
	if( this->size() >= MaxQueueSize)
		return NULL; 

	partids.push_back(partid);

	CJobCommand * job = new CJobCommand(this);
	job->AddJob(_cmsd, jobId, partid);
	push_back(job);
	return job;
}


// This is the root of the jobs tree
void CJobCommands::InitAllJobs(Job *	job) 
{
	//jobids = job->partIds;
	for(int i=0 ; i< job->partIds.size(); i++)
	{
		parts.push_back( (LPCSTR) job->partIds[i]);
		numparts[ (LPCSTR) job->partIds[i]]= 0;
		finishedparts[ (LPCSTR) job->partIds[i]]=0;
		totnumparts[ (LPCSTR) job->partIds[i]]=ConvertString<int>((LPCSTR)job->partQuantity[i],1);
	}
	std::map<std::string, int> partcounts;
	for(int i=0 ; i< job->partIds.size(); i++)
	{
		partcounts[(LPCSTR) job->partIds[i]]=ConvertString<int>((LPCSTR) job->partQuantity[i],9);
	}
	for(std::map<std::string, int>::iterator it=partcounts.begin(); it!= partcounts.end(); it++)
	{		
		for(int i=0; i< (*it).second; i++)
			random_part.push_back((*it).first);
	}

	// Now randomize part making
	std::random_shuffle ( random_part.begin(), random_part.end() );
	// Get Properties that may change from job to job - These are GLOBALS
	// Get KWH Cost
	std::string kwh= job->GetPropertyValue("KWH"); // we will assume cost but should add description tag
	ControlThread::globalCosts["KWH"] = ConvertString<double>(kwh,0.0);   // no cost if not found :(
	GLogger << FATAL << ControlThread::globalCosts["KWH"] << std::endl;

}
void CJobCommands::InitJobsStats(Job *	job) 
{
	static std::string fcnname="CJobCommands::InitJobsStats(Job *	job)  ";

	// For each part, dissect to each Ex resource used, time on each resource
	std::vector<double> dOperationTimeForPart(job->partIds.size(),0.0); // operation time on this line to finish this job
	
	for(int k=0 ; k< cmsd->jobs->size(); k++)
	{
		ATLASSERT (cmsd->jobs->at(k)!=NULL);
		Job * job = (Job *) cmsd->jobs->at(k).get();

		// For each part, dissect to each Ex resource used, time on each resource
		std::vector<double> dOperationTimeForPart(job->partIds.size(),0.0); // operation time on this line to finish this job

		for(int i=0 ; i< job->partIds.size(); i++)
		{
			// This sums up how long a part is to take given a set of resources (using the resource MTTP)
			std::string partid = job->partIds[i];
			std::vector<CResourceHandler * > ResourceHandlers = Factory.GetJobResources(partid);
			for(int k=0 ; k< ResourceHandlers.size(); k++)
			{
				dOperationTimeForPart[i]+= ResourceHandlers[k]->_statemachine->MTTP;
				Stats::Update("PTU" , dOperationTimeForPart[i] , ResourceHandlers[k]->_partStats[partid]);
			}
		}
		double dTotalOperationTime=0.0;
		for(int i=0 ; i< job->partIds.size(); i++)
		{
			std::string partid = job->partIds[i];
			dTotalOperationTime+=dOperationTimeForPart[i] * totnumparts[partid];  // total seconds for all parts

			// each resource will take  # parts to make in job * time/part
			std::vector<CResourceHandler * > ResourceHandlers = Factory.GetJobResources(partid);
			for(int k=0 ; k< ResourceHandlers.size(); k++)
			{
				ResourceHandlers[k]->_partStats[partid].Property("name")= (LPCSTR) ResourceHandlers[k]->_resource->name;
				ResourceHandlers[k]->_partStats[partid]["OT"]=ResourceHandlers[k]->_statemachine->MTTP * totnumparts[partid];
				ResourceHandlers[k]->_partStats[partid]["PSUT"]= 0.0;
				ResourceHandlers[k]->_partStats[partid]["PlannedStandstill"]= 0.0;
				ResourceHandlers[k]->_partStats[partid].Get("PBT")= ResourceHandlers[k]->_partStats[partid]["OT"];
				
			}
		}

		stats.Property("name")="Factory Planned";
		stats.vals["OT"]=dTotalOperationTime;
		stats.vals["PSUT"]=0.0;
		stats.vals["PlannedStandstill"]=0.0;
		stats.vals["PBT"]=dTotalOperationTime;
		
	}
}
//
void CJobCommands::IncPart(std::string partid)
{
	numparts[ partid]= numparts[ partid]+1;
	finishedparts[ partid]=finishedparts[ partid]+1;
}
int CJobCommands::PartsInProcess()
{
	int parts=0;
	for(std::map<std::string,int >::iterator it = finishedparts.begin(); it!=finishedparts.end(); it++)
		parts+=(*it).second;
	return parts;
}

int CJobCommands::AllFinished() 
{			
	for(std::map<std::string,int >::iterator it=finishedparts.begin(); it!=finishedparts.end(); it++)
	{
		if((*it).second < totnumparts[(*it).first])
			return false;
	}
	return true;
}
void  CJobCommands::Newworkorder()
{	
	static std::string fcnname="JobCommands::Newworkorder() ";
	while(IsNewWorkorder()) 
	{
		try {
			if(AllFinished())
				break;
			if(this->size() >0  )
			{
				CJobCommand * job = at(this->size()-1);
				if(job == NULL || job->_ResourceHandlers.size() <=0)
					break;

				CResourceHandler * resource = DOCHECK(job->_ResourceHandlers[0], fcnname +  "Bad resource handler" );
				if(resource->_statemachine->GetState() == "blocked")
				{
					break;  // No new jobs - already blocked...
				}
			}

			int current =  ((int) (this->size()-1) >0) ? this->size()-1 : 0;
			std::string partid = GetPartId(current);
			CJobCommand * job = DOCHECK(AddJob(cmsd, _jobid ,partid), fcnname + "Add job failed");
			job->LoadResources(job->_partid);
		}
		catch(std::exception err) 
		{ 
			OutputDebugString(err.what()); 
		}
	}
}

void CJobCommands::Run(CJobCommands * jobs)
{
	//MaxQueueSize=2;
	_jobid=0;
	m_bRunning=true;
	serviceTime.Start();
	m_Thread = boost::thread(&CJobCommands::process, this, jobs);

}

void CJobCommands::Update() // CResourceHandlers * resourceHandlers)
{

	for(int i=0; i < size() ; i++)
	{
		int _current = at(i)->_currentstep;

		// only check this - if done
		// check next - if done and free - else nothing...
		if(_current>= at(i)->MaxStep() )
		{
			at(i)->orderTime.Stop();
			at(i)->factoryTime.Stop();
			IncPart(at(i)->CurrentPartId());
			// Done with job
			// for now erase

			this->erase(begin() + i);
			partids.erase(partids.begin() + i);

			// Eliminate this part
			continue;
		}
		
		CJobCommand * job = at(i);

		// Check to see if job has not started as
		if(_current < 0 ) 
		{
			if(  job->_ResourceHandlers.size() >0 
				&& job->_ResourceHandlers[0]->_statemachine->CanPush()
				&& job->_ResourceHandlers[0]->_statemachine->GetStateName() != "faulted"
				)
			{ 
				at(i)->_currentstep=0;

				at(i)->factoryTime.Start();
				job->_ResourceHandlers[0]->_statemachine->Push(job);  // start job
				job->_mttp=job->_ResourceHandlers[0]->_statemachine->MTTP;  // reset time to finish - new job
			}
			continue; // maybe reset to zero - no more jobs?
		}
		CResourceHandler *  resource = job->_ResourceHandlers[_current] ; 
		CResourceHandler *  resource1;
		if(resource == NULL) 
			continue;

		// Done with processing at current step
		if(resource ->_statemachine->Current() != NULL && (at(i)-> _mttp <= 0.0) )
		{
			// Done no more steps
			if((_current+1)>= at(i)->MaxStep())
			{
				resource->_statemachine->Pop();  // finish/pop current step
				                           // which is at front of queue
				at(i)->_currentstep++; // increment current step
				i=i-1; // redo this to remove !?
				continue; // skip all processing
			}
			resource1 = job->_ResourceHandlers[_current+1] ;
			if(resource1==NULL)  // should break if probem
					continue;
			if( resource1->_statemachine->CanPush() && resource1->_statemachine->GetStateName() != "faulted" )
			{
				CJobCommand * job = resource->_statemachine->Pop();    // finished current step
				//resource1 -> _mttp =resource1 -> MTTP;  // restart counter resource1 - yes or no
				at(i)->_currentstep++; // incremeent current step
				resource1->_statemachine->Push(job);
				job->_mttp=resource1->_statemachine->MTTP;  // reset time to finish - new job
			}
		}
	}
}


void CJobCommands::process(CJobCommands * jobs)
{
	boost::mutex::scoped_lock lock(_wndMain->_access);

	_dUpdateRateSec=0.0;
	double _dSpeedupRateSec=0.0;
	while(m_bRunning)
	{
		ControlThread::RestartTimer();
		if(!_wndMain->_bFinish)
			Newworkorder();               // done with job - start new work order
		Update() ;					// update job queues 

		double dSpeedup =1.0;
		ControlThread::_dSpeedup=1.0;
		if(_wndMain->_bZip==true)
		{
			dSpeedup =10000.0;
			for(int i=0 ; i<Factory.size(); i++)
			{
				double dUp = Factory[i]->_statemachine->Speedup();
				dSpeedup = MIN(dSpeedup,dUp);
				if(dSpeedup < 0)
				{
					OutputDebugString(StdStringFormat("Calamity neg uptime %s=%8.4f\n", Factory[i]->_statemachine->Name().c_str(), dUp).c_str());
					DebugBreak();
					Factory[i]->_statemachine->Speedup();
				}
			}
			if(dSpeedup == 10000.0) dSpeedup=1.0;
			if(dSpeedup <= 0.0) dSpeedup=1.0;
			ControlThread::_dSpeedup=dSpeedup;
			//OutputDebugString(StdStringFormat("\n Final Speed up %8.4f\n", dSpeedup).c_str());
		}


		_dUpdateRateSec+= 1.0 * ControlThread::_dSpeedup; // one seconds times speed up
		CTimestamp::UpdateSimElapsed(ControlThread::_dSpeedup);// updating time

		Factory.UpdateResourceHandlers();  // update state machine

		boost::thread::yield();

		Reporting::AgentStatus(this, JobStatus, Jobs, DeviceStatus );

		if( (_dUpdateRateSec ) <= _dDeadline)
		{
			::SendMessage(_wndMain->m_hWnd, WM_COMMAND, DISPLAY_MSG,0);
			DoEvents();
		}
		else
		{
			_wndMain->cond.wait(lock);
			::SendMessage(_wndMain->m_hWnd, WM_COMMAND, DISPLAY_MSG,0);
		}

		// Stop - break while loop
		if(_wndMain->_bStopped==true)
			break;
		// Pause until woken up by step or resume
		if(_wndMain->_bPaused==true)
		{
			_wndMain->cond.wait(lock);
			::SendMessage(_wndMain->m_hWnd, WM_COMMAND, DISPLAY_MSG,0);
		}
		//// Create snapshot of operation
		//if(_wndMain->_bSnapshot==true)
		//{
		//	_wndMain->_bSnapshot=false;
		//	Reporting::GenerateHtmlReport(this, ::ExeDirectory() + "Doc.html");
		//	::SendMessage(_wndMain->m_hWnd, DISPLAY_SNAPSHOT,0,0);
		//	::Sleep(500);
		//}
		// Create KPI snapshot of operation
		if(_wndMain->_bKPISnapshot==true)
		{
			_wndMain->_bKPISnapshot=false;
			
			std::string kpiReport = CHtmlTable::CreateHtmlSytlesheetFrontEnd("KPI Precision Sand Casting Saginaw MI")+"</HEAD>";
			kpiReport += KPI::ReportingPlanned();
			kpiReport += KPI::Reporting(stats);
			kpiReport += "</BODY></HTML>\n";

			::WriteFile(::ExeDirectory() + "JobsKpi.html", kpiReport);
			::SendMessage(_wndMain->m_hWnd, DISPLAY_KPISNAPSHOT,0,0);
			::Sleep(500);

		}
		if(AllFinished())
			break;
		if(_wndMain->_bFinish)
		{
			if(size() == 0) // quit when all the current jobs are done
				break;
		}
	}

	//for(int i=0 ; i<_resourceHandlers->size(); i++)
	//{
	//	std::string tmp = _resourceHandlers->at(i)->_statemachine->GenerateReport();
	//	OutputDebugString(StdStringFormat("Resource(%s) Report=%s\n",_resourceHandlers->at(i)->_identifier.c_str(), tmp.c_str() ).c_str())  ;
	//}
	
	//std::map<std::string,double> states;
	//for(int i=0;i<_resourceHandlers->size() ; i++)
	//	_resourceHandlers->at(i)->_statemachine->GenerateStateReport(states );

	serviceTime.Stop();
	Reporting::GenerateHtmlReport(this, ::ExeDirectory() + "Doc.html" );
}
