#include "StdAfx.h"
#include "ResourceHandler.h"
#include "NIST/Config.h"
#include "NIST/StdStringFcn.h"
//#include "NIST/CLogger.h"
#include "NIST/Logger.h"
#include "NIST/AgentCfg.h"
//#include "MTCAgentCmd.h"
#include "MainFrm.h"
#include "HtmlTable.h"

extern CMainFrame * _wndMain;


#include "EquationSolver.h"
using namespace EquationHelper;
using std::size_t;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::ios;
typedef EquationSolver ES;


//#define DOCHECK(X,Y) ((X!=NULL) ? X : throw std::exception((fcnname+ #Y).c_str()) )
template<typename T>
T DOCHECK(T x, std::string y) 
{
	if (x!=NULL) 
		return x ;
	
	throw std::exception(y.c_str());
	return NULL;
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
void CResourceHandlers::UpdateResourceHandlers()
{	
	for(int i=0 ; i<size(); i++)
	{
		CResourceHandler *  _resourceHandler = at(i);
		if( _resourceHandler == NULL)
			continue;
		_resourceHandler->_statemachine->Cycle();
	}
}

CResourceHandler * CResourceHandlers::Find(std::string name)
{
	for(int i=0 ; i<size(); i++)
		if( std::string((LPCSTR) this->at(i)->_resource->name) == name) 
			return at(i); 
	return NULL; 
}
void CResourceHandlers::CreateResourcesByPart(CCMSDIntegrator * cmsd)
{
	static std::string fcnname="CResourceHandlers::CreateResourcesByPart(CCMSDIntegrator * cmsd) ";
	for(int k=0 ; k< cmsd->jobs->size(); k++)
	{
		ATLASSERT (cmsd->jobs->at(k)!=NULL);
		Job * job = (Job *) cmsd->jobs->at(k).get();
		for(int i=0 ; i< job->partIds.size(); i++)
		{
			std::string partid = job->partIds[i];
			Stats stat;
			try {
				// Find all resource and then MTP for each resource to make part
				Part * part = DOCHECK(cmsd->FindPartById(partid.c_str()),"Bad part lookup") ;
				ProcessPlan * processplan  =  DOCHECK(cmsd->FindProcessPlanById(part->processplanidentifier), "Bad processplan ");
				std::vector< CResourceHandler * > resources;
				for(int j=0;j< 	processplan->steps.size(); j++)
				{
					Process *process = processplan->processes[j];
					Cell * cell= DOCHECK(cmsd->FindCellById((LPCSTR)  process->resourcesRequired[0]), "Bad cell lookup ");

					Resource * r = DOCHECK(cmsd->FindResourceById((LPCSTR) cell->resourceIds[0]), "Bad Resource lookup ");
					std::string name = (LPCSTR) r->name;
					if(name.empty() || name == "None")  // something wrong
						name=r->identifier; 

					CResourceHandler * _resourceHandler = new CResourceHandler(r, cmsd);
					_resourceHandler->_identifier = (LPCSTR) r->identifier;
					_resourceHandler->_statemachine->SetStateMachineName(name);
					resources.push_back(_resourceHandler);
					this->push_back(_resourceHandler);
				}
				_resourceByPartStats[partid]=resources;
			} 
			catch(std::exception err) 
			{ 
				DebugBreak();
				OutputDebugString(err.what()); 
			} 
		}
	}
}