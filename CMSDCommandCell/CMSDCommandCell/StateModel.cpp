// This software was developed by U.S. Government employees as part of
// their official duties and is not subject to copyright. No warranty implied 
// or intended.

#include "StdAfx.h"
#include "CmdCell.h"
#include "NIST/Config.h"
#include "NIST/StdStringFcn.h"
//#include "NIST/CLogger.h"
#include "NIST/Logger.h"
//#include "NIST/AgentCfg.h"
//#include "MTCAgentCmd.h"
#include "MainFrm.h"
#include "KPI.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Sim StateModel - off, idle, starved, blocked, running, faulted

CDESMonitor::CDESMonitor()
{

	_statemachine.push_back(StateMachineTuple(off,init,ready,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Init, this))); 
//	_statemachine.push_back(StateMachineTuple(ready,load, loaded,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Load, this))); 
	_statemachine.push_back(StateMachineTuple(ready,run, running,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Run, this))); 
	_statemachine.push_back(StateMachineTuple(stopped,run, running,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Reset, this))); 
	_statemachine.push_back(StateMachineTuple(running,stop, stopped,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Stop, this))); 
	//_statemachine.push_back(StateMachineTuple(running,hold, interrupted,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Hold, this))); 
	_statemachine.push_back(StateMachineTuple(running, done, finished,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Done, this))); 
	_statemachine.push_back(StateMachineTuple(interrupted,run, running,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Resume, this))); 

	_statemachine.push_back(StateMachineTuple(any,block, blocked,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Blocked, this))); 
	_statemachine.push_back(StateMachineTuple(blocked,run, running,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Running, this))); 
	_statemachine.push_back(StateMachineTuple(starved,run, running,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Running, this))); 
	_statemachine.push_back(StateMachineTuple(faulted,run, running,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Running, this))); 
	_statemachine.push_back(StateMachineTuple(any,run, running,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Running, this))); 
	_statemachine.push_back(StateMachineTuple(any,starved, starved,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Running, this))); 

	_statemachine.push_back(StateMachineTuple(any,quit, "myExit",  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Exit, this))); 
	_statemachine.push_back(StateMachineTuple(any,fail, faulted,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Fail, this))); 
	_statemachine.push_back(StateMachineTuple(any,estop, estopped,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Stop, this))); 
	_statemachine.push_back(StateMachineTuple(any,reset, ready,  boost::bind(&MyClass::NullCondition, this),	boost::bind(&MyClass::Reset, this))); 

	_stateMap[off]=boost::bind(&MyClass::Off, this);
	_stateMap[ready]=boost::bind(&MyClass::Ready, this);
	_stateMap[running]=boost::bind(&MyClass::Running, this);
	_stateMap[stopped]=boost::bind(&MyClass::Stopped, this);
	_stateMap[interrupted]=boost::bind(&MyClass::Interrupted, this);
	_stateMap[faulted]=boost::bind(&MyClass::Faulted, this);
	_stateMap["myExit"]=boost::bind(&MyClass::Exit, this);
	_stateMap["blocked"]=boost::bind(&MyClass::Blocked, this);
	_stateMap["starved"]=boost::bind(&MyClass::Starved, this);

	_dUpdateRateSec=1.0;  // assume updates of 1 second
	Setup();

}
void CDESMonitor::Setup()
{
	stats.nBlockedTime=0.0;
	stats.nStarvedTime=0.0;
	stats.nDownTime=0.0;
	stats.nProductionTime=0.0;
	stats.nOffTime=0.0;
	stats.nRepairTime=0.0;
	stats.nIdleTime=0.0;
	_mtbf=MTBF;
	_mttr=MTTR;
	stats.nTotalParts=stats.nGoodParts=0;

}
void CDESMonitor::Off()
{
	LogMessage(name); LogMessage("Off\n");
	stats.nOffTime+= this->UpdateRate()*1000.00;
}
void CDESMonitor::Running()
{ 

		//OutputDebugString(StdStringFormat("%s Running Queue Current = %X %x\n", name.c_str(), Current(), ConvertToString(_mttp).c_str()).c_str());
	if(this->MTBF>0) 
	{
		//_mtbf-= _dUpdateRateSec;
		_mtbf-= dMtbf;
	}

	if(Current()!=NULL && Current()->_mttp>0 && Current()!=NULL) 
		stats.nProductionTime+= dMtp;
	else
		stats.nIdleTime+= dRMtp;

	//// Now in faulted state??
	if(MTBF>0  && _mtbf<=0 )
	{
		_mttr=MTTR-dRMtbf  ;  // add on extra time to repair
		SyncEvent("fail");
		stats["FE"]=stats["FE"]+1;
		return;
	}
	
	if(Current()!=NULL && MTTP>0) 
	{
		Current()->_mttp-=dMtp;
	}
		// Add up time in production state
	
	if(Current()!=NULL && Current()->_mttp<=0 && MTTP>0) // nwas blocking when 0 MTP was set
	{
		this->SyncEvent("block");
		return;
	}
	else if(Current()!=NULL) 
	{
		return;
	}
	if(Current()==NULL)
	{
		SyncEvent("starved");
		return;
	}
}

void CDESMonitor::Blocked()
{
	//OutputDebugString(StdStringFormat("%s Blocked Queue Current = %X %x\n", name.c_str(), Current(), ConvertToString(nBlockedTime).c_str()).c_str());
	stats.nBlockedTime+= _dUpdateRateSec;
	if(Current() != NULL && (Current()->_mttp>0 ) )  // item to process...
		this->SyncEvent("run");
	if(Current() == NULL)
		this->SyncEvent("starved");
}
void CDESMonitor::Starved()
{
	//OutputDebugString(StdStringFormat("%s Starved Queue Current = %X %x\n", name.c_str(), Current(), ConvertToString(nBlockedTime).c_str()).c_str());
	stats.nStarvedTime+= _dUpdateRateSec;

	if(Current() != NULL) 
		this->SyncEvent("run");
}
void CDESMonitor::Faulted()
{ 
	//OutputDebugString(StdStringFormat("%s Faulted Downtime=%8.2f RepairTime=%8.2f Mttr =%8.2f Mtbf =%8.2f\n",name.c_str(), nDownTime,nRepairTime,_mttr ,_mtbf ).c_str() );
	//  should never be here if MTTR <=0
	if(MTTR>0) 
		_mttr-= _dUpdateRateSec;;
	//_mtbf-= _dUpdateRateSec;  // subtract second til next  fault
	// Add up time in faulted state
	stats.nDownTime+= _dUpdateRateSec; // assume seconds
	stats.nRepairTime+= _dUpdateRateSec;  // assume seconds
	// Add up utilites in faulted state? Or just add to preprocess step

	// Done in faulted state?
	if(_mttr<=0)
	{
		_mtbf=MTBF;  // start faulted wait over - is MTBF< MTTR - YIKES
		SyncEvent("run");
	}
	
}	

std::string CDESMonitor::ToString()
{ 
	std::string tmp =  StdStringFormat("%s State  %s Rate=%8.2f In:%d Job:%x TTP=%8.2f\n", this->_statemachinename.c_str(), GetStateName().c_str(), this->UpdateRateSec(),
		size(),  Current(),Current()->_mttp);
	return tmp;
}

double  CDESMonitor::Speedup()
{
	OutputDebugString(StdStringFormat("CDESMonitor::Speedup() %s\n",Name().c_str()).c_str());
	double dSpeedup=99999.0;
	if(this->GetStateName() == "faulted" ) 
	{
		if(_mttr > 0 && MTBF>0)
			dSpeedup=_mttr;
	}
	else if(GetStateName() == "running"   )
	{
		if(_mtbf>0 && MTBF>0) 
			dSpeedup=_mtbf;
		if( Current()  != NULL && Current()->_mttp != 0.0) 
			dSpeedup=MIN(dSpeedup, Current()->_mttp);
	}
	else if(GetStateName() == "blocked" || GetStateName() == "starved")
	{
		dSpeedup=100000.0;
		return dSpeedup;
	}
	return dSpeedup;
}

void CDESMonitor::Postprocess()
{
	//OutputDebugString(StdStringFormat("%s", this->Name().c_str()).c_str());

	for(int i=0; i< _costfcns.size() ; i++)
	{
		if(get<0>(_costfcns[i]) == "any")
		{
			get<4>(_costfcns[i])=get<4>(_costfcns[i]) + _dUpdateRateSec * get<5>(_costfcns[i]);
		}
		if(get<0>(_costfcns[i]) == GetState()) // looking for faulted state
		{
			get<4>(_costfcns[i])=get<4>(_costfcns[i]) + _dUpdateRateSec * get<5>(_costfcns[i]);
		}

	}
}
void CDESMonitor::Preprocess()
{
	_dUpdateRateSec = UpdateRateSec();

	dMtbf= MIN(_dUpdateRateSec,_mtbf);
	dRMtbf=  MIN(_dUpdateRateSec-dMtbf,_mtbf-dMtbf);

	dMtp= _dUpdateRateSec;
	if(Current() != NULL)
		dMtp = MIN(dMtp,Current()->_mttp);
	else dMtp=0.0;

	dRMtp = POSMIN(_dUpdateRateSec-dMtp,0);

	dMttr = MIN(_dUpdateRateSec,_mttr);;
	dRMttr=  MIN(_dUpdateRateSec-dMttr,_mttr-dMttr);

	double dPT;
	if(Current() != NULL)
		dPT =Current()->_mttp;
	else
		dPT = 0.0;
	
	OutputDebugString(StdStringFormat("Preprocess %s TP=%8.2f TF=%8.2f Mtbf=%8.2f RMtbf=%8.2f Mtp=%8.2f RMTp=%8.2f \n",Name().c_str(), dPT ,_mtbf,dMtbf,dRMtbf,dMtp, dRMtp).c_str());

	Stats::Update("TotalTime", UpdateRateSec(), stats);

	if( Current() !=NULL &&  pLastCurrent!=Current())
	{
	 	Stats &partStats= _partStats[ Current()->_partid];
	    Stats::Update("TotalParts", 1.0, Current()->stats,partStats, stats);
	    Stats::Update("GoodParts", 1.0, Current()->stats,partStats, stats);

	};

	pLastCurrent=Current();
	Stats::Update(StateStr(GetState()), _dUpdateRateSec, stats);
	if( Current() !=NULL)
	{
	 	Stats partStats= _partStats[ Current()->_partid];
		Stats::Update(StateStr(GetState()), _dUpdateRateSec, Current()->stats,Current()->GetStepStat(),partStats);
	    Stats::Update("TotalTime", _dUpdateRateSec, Current()->stats,Current()->GetStepStat(),partStats);
		 // fix me those on queue - are idle, waiting
		 for(int i=1; i< Queue<>::size(); i++)
			Stats::Update("idle", _dUpdateRateSec,  at(i)->stats,Current()->GetStepStat(),partStats);

	
		
	}
}


std::string CDESMonitor::GenerateReport()
{
	std::string tmp;
	tmp+= ToString();
	tmp+=StdStringFormat("\t Down Time =%8.2f\n", stats.nDownTime);
	tmp+=StdStringFormat("\t Blocked Time =%8.2f\n", stats.nBlockedTime);
	tmp+=StdStringFormat("\t Starved Time =%8.2f\n", stats.nStarvedTime);
	tmp+=StdStringFormat("\t Repair Time =%8.2f\n", stats.nRepairTime);
	tmp+=StdStringFormat("\t Production Time =%8.2f\n", stats.nProductionTime);
	tmp+=StdStringFormat("\t Off Time =%8.2f\n", stats.nOffTime);
	return tmp;
}

void CDESMonitor::GenerateStateReport(std::map<std::string,double> &states, double dDivisor )
{

	states["down"]=0.0; 
	states["blocked"]=0.0; 
	states["starved"]=0.0; 
	//states["repair"]=0.0; 
	states["production"]=0.0; 
	states["off"]=0.0; 

	if(dDivisor!=1.0)
		dDivisor= (stats.nBlockedTime+stats.nStarvedTime+stats.nDownTime+stats.nProductionTime+stats.nOffTime)/100.0;

	states["down"]+= stats.nDownTime/dDivisor;
	states["blocked"]+= stats.nBlockedTime/dDivisor;
	states["starved"]+= stats.nStarvedTime/dDivisor;
	//states["repair"]+= nRepairTime/dDivisor;
	states["production"]+= stats.nProductionTime/dDivisor;
	states["off"]+= stats.nOffTime/dDivisor;
}

std::string CDESMonitor::GenerateCSVHeader(std::string units)
{
	// FIXME: this will have to be smarter
	 return StdStringFormat("Down<BR>(%s)</BR>,Blocked<BR>(%s)</BR>,Starved<BR>(%s)</BR>,Production<BR>(%s)</BR>,Off<BR>(%s)</BR>",
		 units.c_str(), units.c_str(), units.c_str(),units.c_str(),units.c_str());
}
std::string CDESMonitor::GenerateCSVTiming(double divider)
{
	std::string tmp;
	tmp+=StdStringFormat("%8.2f,", stats.nDownTime/divider);
	tmp+=StdStringFormat("%8.2f,", stats.nBlockedTime/divider);
	tmp+=StdStringFormat("%8.2f,", stats.nStarvedTime/divider);
//	tmp+=StdStringFormat("%8.2f,", nRepairTime/divider);
	tmp+=StdStringFormat("%8.2f,", stats.nProductionTime/divider);
	tmp+=StdStringFormat("%8.2f", stats.nOffTime/divider);
	return tmp;
}


std::string CDESMonitor::GenerateCosts(std::string prepend, std::string postpend, std::string state)
{
	std::string tmp;
	// Units, name, state, time, cost
	for(int i=0; i< _costfcns.size(); i++)
	{
		std::string entry;
		std::string costunit = get<2>(_costfcns[i]) ;
		entry+=get<2>(_costfcns[i]) + "," ; // units
		entry+=get<1>(_costfcns[i]) + ",";  // name
		entry+=get<0>(_costfcns[i])+ ",";  // state
		entry+=ConvertToString(get<4>(_costfcns[i]))+ ",";  // time
		entry+="$";
		if(globalCosts.find(costunit) != globalCosts.end())
			entry+=ConvertToString(get<4>(_costfcns[i] ) * globalCosts[costunit]);
		else
			entry+=ConvertToString(get<4>(_costfcns[i]));

		if(get<0>(_costfcns[i]) != state)
			continue;
		tmp+=prepend;
		tmp+=entry;
		tmp+=postpend;
	}
	return tmp;
}
std::string CDESMonitor::GenerateTotalCosts(std::string prepend, std::string postpend)
{
	std::string tmp;
	double cost=0.0;
	// Units, name, state, time, cost
	for(int i=0; i< _costfcns.size(); i++)
	{
		std::string costunit = get<2>(_costfcns[i]) ;
		if(globalCosts.find(costunit) != globalCosts.end())
			cost+=get<4>(_costfcns[i] ) * globalCosts[costunit];
		else
			return ",";
	}
	tmp=prepend;
	tmp+=StdStringFormat("$%8.2f,",cost);
	tmp+=postpend;
	return tmp;
}
/**

	Electrical Voltage
	Electrical  HP
	Electrical  Motor kW
	Electrical  Load kW
	Electrical  kVA
	Electrical Demand Factor
	Electrical  Demand kVA
	Electrical  Circuit Size

	Natual Gas PSI
	Natual Gas CFG
	Natual Gas  Size

	Water City GPM
	Water Plant GPM
	Water Chilled GPM
	Water Size

	Hydraulics Unit
	Hydraulics PSI	
	Hydraulics GPM	
	Hydraulics Size

	Air Dry CFM	
	Air Plant CFM
	Air PSI	
	Air	Size

	Tea Piping

	Resin Piping

	Steam

	Pattern Spray



	#1 LINE
Cast Line 1 Entry Elevator Upper Conveyor
Cast Line 1 Entry Elevator Lower Conveyor
Cast Line 1 Entry Elevator Hydraulic Unit Pump
Cast Line 1 Entry Elevator Hydraulic Unit Recirc Pump

Cast Line 1 Double Deck Entry Upper Conveyor #1
Cast Line 1 Double Deck Entry Upper Conveyor #2
Cast Line 1 Double Deck Entry Lower Conveyor #1
Cast Line 1 Double Deck Entry Lower Conveyor #2

Cast Line 1 Lift-Locate Unit W/Stop

Cast Line 1 Chill Insert Station Upper Elevator Conveyor
Cast Line 1 Chill Insert Station Lower Elevator Conveyor
Cast Line 1 Chill Insert Station Exit Conveyor W/Blow Off
Cast Line 1 Chill Insert Station Hydraulic Unit Pump
Cast Line 1 Chill Insert Station Hydraulic Unit Recirc Pump

Cast Line 1 Cover Insertion Infeed Conveyor
Cast Line 1 Cover Insertion Insertion Conveyor
Cast Line 1 Cover Insertion Exit Conveyor

Cast Line 1 Load Grantry X-Axis Travel
Cast Line 1 Load Grantry Y-Axis Travel
Cast Line 1 Unload Grantry X-Axis Travel
Cast Line 1 Unload Grantry Y-Axis Travel
Cast Line 1 Load/Unload Grantry Hydraulic Unit Pump
Cast Line 1 Load/Unload Grantry Hydraulic Unit Recirc Pump

Cast Line 1 Outfeed Conveyor #1
Cast Line 1 Outfeed Conveyor #2
Cast Line 1 Outfeed Conveyor #3

Cast Line 1 Chill Extraction Station Inlet Conveyor
Cast Line 1 Chill Extraction Station Upper Elevator Conveyor
Cast Line 1 Chill Extraction Station Lower Elevator Conveyor
Cast Line 1 Chill Extraction Station Hydraulic Unit Pump
Cast Line 1 Chill Extraction Station Hyd. Unit Recirc Pump

Cast Line 1 Double Deck Outfeed Upper Conveyor #1
Cast Line 1 Double Deck Outfeed Upper Conveyor #2
Cast Line 1 Double Deck Outfeed Lower Conveyor #1
Cast Line 1 Double Deck Outfeed Lower Conveyor #2

Cast Line 1 Stuck Chill Reject Station
Cast Line 1 Double Deck Exit Upper Conveyor #1
Cast Line 1 Double Deck Exit Upper Conveyor #2
Cast Line 1 Double Deck Exit Lower Conveyor #1
Cast Line 1 Double Deck Exit Lower Conveyor #2
Cast Line 1 Exit Elevator Hydraulic Unit Pump
Cast Line 1 Exit Elevator Hydraulic Unit Recirc Pump
Cast Line 1 Rollover Main Panel
Cast Line 1 Rollover Index Table
Cast Line 1 Rollover Hydraulic Motor #1
Cast Line 1 Rollover Hydraulic Motor #2
Cast Line 1 Rollover Recirc Pump
Cast Line 1 EM Pump Preheat Oven
Cast Line 1 EM Pump Cooling Fan Control Panel
Cast Line 1 EM Pump Cooling Motor #1
Cast Line 1 EM Pump Cooling Motor #2
Cast Line 1 EM Pump Main Control Panel

*/

//double CDESMonitor::GetUpdateFactor()
//{
//
//	std::string  state = GetState();
//	double maxWait=10000.0;
//	if(state == "starved")
//	{
//		maxWait=1.0;
//	}
//	else if(state == "blocked")
//	{
//		maxWait=1.0;
//	}
//	else if(state == "running")
//	{
//		if(MTTP>0 && currentJob!=NULL) 
//			maxWait=MIN(maxWait,_mttp);
//		if(MTBF>0) 
//			maxWait=MIN(maxWait,_mtbf);
//		if(currentJob!=NULL && _mttp<=0)
//			maxWait=MIN(maxWait,_mttp);
//		if(currentJob==NULL && inqueue.size() > 0)
//			maxWait=1.0;
//
//	}
//	else if(state == "faulted")
//	{
//		maxWait=MIN(maxWait,_mttr); 
//	}
//	else
//	{
//		maxWait=1.0;
//	}
//	return maxWait;
//}