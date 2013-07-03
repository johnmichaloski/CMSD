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
extern CMainFrame * _wndMain;
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
		orderTime.Start();
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

	part = _cmsd->FindPartById( partid.c_str());
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
void CJobCommand::LoadResources(CResourceHandlers *resources)
{
	for(int i=0; i< _ResourceRequired.size();i++)
	{
		_ResourceHandlers.push_back(resources->Find(_ResourceRequired[i]));
		_TimePerStep.push_back(CTimestamp());
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CJobCommands::CJobCommands(CCMSDIntegrator * _cmsd) 
{
	//agentconfig= _agentconfig;
	cmsd= _cmsd;
	_resourceHandlers=NULL;
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
	while(IsNewWorkorder()) 
	{
		if(AllFinished())
			break;
		if(this->size() >0  )
		{
			CJobCommand * job = at(this->size()-1);
			if(job == NULL || job->_ResourceHandlers.size() <=0)
				break;

			CResourceHandler * resource = at(this->size()-1)->_ResourceHandlers[0];
			if(resource == NULL)
				break;
			if(resource->_statemachine->GetState() == "blocked")
			{
				break;  // No new jobs - already blocked...
			}
		}

		int current =  ((int) (this->size()-1) >0) ? this->size()-1 : 0;
		std::string partid = GetPartId(current);
		CJobCommand * job = AddJob(/*agentconfig,*/ cmsd, _jobid ,partid);  // error FIXME
		if(job==NULL)
			break;

		job->LoadResources(_resourceHandlers);

		//std::string d= back()->ToString();
		//OutputDebugString(d.c_str());

	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CResourceHandler::~CResourceHandler(void)
{
}
CResourceHandler::CResourceHandler(Resource * r, CCMSDIntegrator * cmsd)
{
	job=NULL;
	_cmsd=cmsd;
	_resource=r;
	_statemachine = new CDESMonitor;

	std::string name = (LPCSTR) r->name;

	std::string mtbf= r->GetPropertyValue("Mtbf");
	std::string mttr= r->GetPropertyValue("Mttr");
	std::string mttp= r->GetPropertyValue("Mttp");

	std::string mtbfunits= r->GetPropertyUnits("Mtbf");
	std::string mttrunits= r->GetPropertyUnits("Mttr");
	std::string mttpunits= r->GetPropertyUnits("Mttp");

	std::string inqMax= r->GetPropertyValue("InQueue");
	_statemachine->MaxSize()= ConvertString<int>(inqMax,1); 

	_statemachine->SetMTBF(ConvertString<double>(mtbf.c_str(), -1.0) * 60.0);// min to sec
	_statemachine->SetMTTR(ConvertString<double>(mttr.c_str(), -1.0) * 60.0);// min to sec
	_statemachine->SetMTTP(ConvertString<double>(mttp.c_str(), -1.0));// sec
	_statemachine->Setup();

	for(int i=0; i<  r->properties.size(); i++)
	{
		std::string name = r->GetPropertyName((LPCSTR) r->properties[i]);
		
		std::string sPropDesc= r->GetProperty(i, 3);  //r->GetPropertyDescription(name);
		if(Trim(sPropDesc) == "COST")
		{
			// Error - can have multiple costs and properties with same name
			std::string sPropUnit= r->GetProperty(i, 2); //GetPropertyUnits(name);
			std::string sPropValue= r->GetProperty(i, 1);   // GetPropertyValue(name);
			std::string  sCostFactor= sPropUnit.substr(0,sPropUnit.find(":"));
			std::string  sCostState = sPropUnit.substr(sPropUnit.find(":")+1);

			std::string solvedvalue=ES::solve(sPropValue, 4);
			double value = ConvertString<double>(solvedvalue,0.0);
			// skip costs < 0
			if(value < 0.0)
				continue;
			_statemachine->AddCostFunction(CostFcnTuple(sCostState, name,sCostFactor, sPropDesc, 0.0, value));

			//OutputDebugString(StdStringFormat("Property Name = %s\n", name.c_str()).c_str());
		}
	}


	_statemachine->Trigger("init");
	_statemachine->Trigger("run");
}
void CJobCommands::Run(CJobCommands * jobs, CResourceHandlers *  resourceHandlers)
{
	//MaxQueueSize=2;
	_jobid=0;
	m_bRunning=true;
	serviceTime.Start();
	m_Thread = boost::thread(&CJobCommands::process, this, jobs, resourceHandlers );

}
std::string CJobCommands::GenerateHtmlReport()
{
	CHtmlTable htmlTable; 
	std::string html = CHtmlTable::CreateHtmlSytlesheetFrontEnd("Precision Sand Casting Saginaw MI");;
	//html += htmlTable.HtmlGoogleChart(states);
	html += htmlTable.HtmlRaphaeleChart( );

	//html+= htmlTable.HtmlRaphaeleChartData(states );


	html += "</HEAD>\n";

	//std::string html = CHtmlTable()::CreateHtmlDocument();
	std::string header = "Machine," + _resourceHandlers->at(0)->_statemachine->GenerateCSVHeader("Minutes") + ",Utility<BR>Costs</BR>,Description";
	std::string alignment = ",right,right,right,right,right,right,right,";
	htmlTable.SetAlignment(alignment);
	htmlTable.SetHeaderColumns( header);

	for(int i=0;i<_resourceHandlers->size() ; i++)
	{
		std::string html1=_resourceHandlers->at(i)->_statemachine->Name() + ","; ///???
		html1+= _resourceHandlers->at(i)->_statemachine->GenerateCSVTiming(60.00) + "," ;
		html1+= _resourceHandlers->at(i)->_statemachine->GenerateTotalCosts("","");
		html1+=  (LPCSTR) _resourceHandlers->at(i)->_resource->description;
		htmlTable.AddRow(header, html1);
	}

	CHtmlTable htmlRawTable; 
	std::string headerRaw = "Machine,Blocked,Starved,Down,Production,Off";
	htmlRawTable.SetHeaderColumns( headerRaw);
	std::map<std::string,double> rawstates;
	for(int i=0;i<_resourceHandlers->size() ; i++)
	{
		std::string html3;
		_resourceHandlers->at(i)->_statemachine->GenerateStateReport(rawstates,1.0 );
		html3+=_resourceHandlers->at(i)->_statemachine->Name() + ",";
		html3+=StdStringFormat("%8.4f,%8.4f,%8.4f,%8.4f,%8.4f\n",
			rawstates["blocked"],rawstates["starved"],rawstates["down"],rawstates["production"],rawstates["off"]);
		htmlRawTable.AddRows(headerRaw, html3);
	}

	CHtmlTable kpiTable; 
	std::string kpiheader = "Machine,MTBF,MTTR,INQ, OUTQ";
	kpiTable.SetHeaderColumns( kpiheader);
	for(int i=0;i<_resourceHandlers->size() ; i++)
	{
		std::string machine=_resourceHandlers->at(i)->_statemachine->Name() + ",";
		machine+= StdStringFormat("%8.4f,",_resourceHandlers->at(i)->_statemachine->MTBF  );
		machine+= StdStringFormat("%8.4f,",_resourceHandlers->at(i)->_statemachine->MTTR  );
		machine+= StdStringFormat("%d,",_resourceHandlers->at(i)->_statemachine->MaxSize()  );
		kpiTable.AddRows(kpiheader, machine);
	}

	CHtmlTable htmlTable2; 
	std::string header2 = "Machine,Units,Name,State,Time,Cost";
	htmlTable2.SetHeaderColumns( header2);
	for(int i=0;i<_resourceHandlers->size() ; i++)
	{
		std::string machine=_resourceHandlers->at(i)->_statemachine->Name() + ",";
		std::string html2= _resourceHandlers->at(i)->_statemachine->GenerateCosts(machine,"\n") ;
		htmlTable2.AddRows(header2, html2);
	}

	html+=	"<h1>Precision Casting</h1>\n";
	for(int i=0; i < this->at(0)->_cmsd->documentation->size(); i++)
		html+= "<p> " + UrlDecode (   (LPCSTR) ((Documentation *) this->at(0)->_cmsd->documentation->at(i).get())->description);
	html+=	"<p>\n";

	html += htmlRawTable.CreateHtmlTable();
	html+=	"<p>\n";
	html += kpiTable.CreateHtmlTable();


	html+=	"<h2>Casting Line 1 Performance</h2>\n";

	for( std::map<std::string, int >::iterator it = finishedparts.begin(); it!= finishedparts.end(); it++)
		html+=StdStringFormat("<BR> Part %s Number Made = %d\n" , (*it).first.c_str(), (*it).second).c_str();
	html+=	StdStringFormat("<br>Time elapsed %8.4f in hours\n", _dUpdateRateSec/3600.0);
	html+=	StdStringFormat("<br>Time elapsed %8.4f in minutes\n", _dUpdateRateSec/60.0);
	html+=	StdStringFormat("<br>Time elapsed %8.4f in seconds\n", _dUpdateRateSec);
	
	html += htmlTable.CreateHtmlTable();
	html+="<br>";
	html+=	"<h1>Casting Line 1 Utility Costs By Item</h1>\n";
	html += CHtmlTable::HtmlStateInfo();
	html+="<br>";
	html += htmlTable2.CreateHtmlTable();

	html+="<div id=\"holder\";\"></div>\n";
	//html +=" var chart = new google.visualization.PieChart(document.getElementById('chart_div'));\n";
	//html +=" chart.draw(data, options);\n";

		//////////////////////////////////////////////////////////////////////////
	// Make table of pie charts of performance states
	HtmlTableMaker Table(3);
	for(int i=0;i<_resourceHandlers->size() ; i++)
	{
		std::string machine=_resourceHandlers->at(i)->_statemachine->Name() + ",";
		std::map<std::string,double> states;
		 _resourceHandlers->at(i)->_statemachine->GenerateStateReport(states,1000.0 );;
		Table.AppendTableCell(Raphael::InlinePieChart(states, machine + " State Use"));
	}
	html+=Table.MakeTable();

	html+= "</BODY>\n</HTML>\n";

	WriteFile(::ExeDirectory() + "Doc.html", html);
	return html;
}


void CJobCommands::Update(CResourceHandlers * resourceHandlers)
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
			if(  _resourceHandlers->size() >0 
				&& _resourceHandlers->at(0)->_statemachine->CanPush()
				&& _resourceHandlers->at(0)->_statemachine->GetStateName() != "faulted"
				)
			{ 
				at(i)->_currentstep=0;
				at(i)->factoryTime.Start();
				_resourceHandlers->at(0)->_statemachine->Push(job);  // start job
				job->_mttp=_resourceHandlers->at(0)->_statemachine->MTTP;  // reset time to finish - new job
			}
			continue; // maybe reset to zero - no more jobs?
		}
		CResourceHandler *  resource = resourceHandlers->FindResourceHandler(_current) ; 
		CResourceHandler *  resource1;
		if(resource == NULL) 
			continue;

		// Done with processing at current step
		if(resource ->_statemachine->Current() != NULL && (at(i)-> _mttp <= 0) )
		{
			// Done no more steps
			if((_current+1)>= at(i)->MaxStep())
			{
				resource->_statemachine->Pop();  // finish/pop current step
				                           // which is at front of queue
				at(i)->_currentstep++; // increment current step
				continue; // skip all processing
			}
			resource1 = resourceHandlers->FindResourceHandler(_current+1) ;
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


void CJobCommands::process(CJobCommands * jobs, CResourceHandlers *  resourceHandlers)
{
	boost::mutex::scoped_lock lock(_wndMain->_access);

	_dUpdateRateSec=0.0;
	double _dSpeedupRateSec=0.0;
	_resourceHandlers=resourceHandlers;
	while(_resourceHandlers==NULL) ::Sleep(100);
	while(m_bRunning)
	{
		ControlThread::RestartTimer();
		Newworkorder();               // done with job - start new work order
		Update(resourceHandlers);  // update queues to resource
		resourceHandlers->UpdateResourceHandlers();  // update state machine

		// Good example specialized logging per Resource in FurLogger
		//CResourceHandler * resourceHandler= _resourceHandlers->Find("LINE1_PS_CAST1_FUR1");
		//if(resourceHandler!=NULL)
		//{
		//	//std::string dump;		
		//	//dump+= resourceHandler-> _statemachine->GenerateReport();
		//	//FurLogger.LogMessage(StdStringFormat("Resource(%s) Report=%s\n",resourceHandler->_identifier.c_str(), dump.c_str() ) );;
		//}

		//for(std::map<std::string, int > ::iterator it = finishedparts.begin() ; it!=finishedparts.end(); it++)
		//	OutputDebugString(StdStringFormat("END Total Number Parts %s =%d\n", ((*it).first).c_str(), finishedparts[(*it).first] ).c_str())  ;

		_dUpdateRateSec+= 1.0 * ControlThread::_dSpeedup; // one seconds times speed up

		boost::thread::yield();
		// Break when shift time exceeded
		//if( (_dUpdateRateSec ) > _dDeadline)
		//	_wndMain->cond.wait(lock);


		//		if(_wndMain->_nLoopCounter<0) 
		if( (_dUpdateRateSec ) <= _dDeadline)
		{
			//_wndMain->_nLoopCounter--;
			::PostMessage(_wndMain->m_hWnd, WM_COMMAND, DISPLAY_MSG,0);
			::Sleep(500);

		}
		else
		{
			_wndMain->cond.wait(lock);
			::SendMessage(_wndMain->m_hWnd, WM_COMMAND, DISPLAY_MSG,0);
		}
		//else
		//{
		//	_wndMain->_nLoopCounter--;
		//	::PostMessage(_wndMain->m_hWnd, WM_COMMAND, DISPLAY_MSG,0);
		//	::Sleep(500);
		//	//::SendMessage(_wndMain->m_hWnd, WM_COMMAND, DISPLAY_MSG,0);
		//	//boost::thread::yield();
		//}

		// Stop - break loop
		if(_wndMain->_bStopped==true)
			break;
		if(_wndMain->_bPaused==true)
		{
			_wndMain->cond.wait(lock);
			::SendMessage(_wndMain->m_hWnd, WM_COMMAND, DISPLAY_MSG,0);
		}
		if(_wndMain->_bSnapshot==true)
		{
			_wndMain->_bSnapshot=false;
			GenerateHtmlReport();
			::SendMessage(_wndMain->m_hWnd, DISPLAY_SNAPSHOT,0,0);
			::Sleep(500);

		}
		if(AllFinished())
			break;
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
	GenerateHtmlReport();
}