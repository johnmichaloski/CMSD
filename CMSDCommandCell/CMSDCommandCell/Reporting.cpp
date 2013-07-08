#include "StdAfx.h"
#include "Reporting.h"
#include "HtmlTable.h"


#include "MainFrm.h"
extern CMainFrame * _wndMain;


Reporting::Reporting(void)
{
}


Reporting::~Reporting(void)
{
}



void Reporting::AgentStatus(CJobCommands * jobs, std::string &JobStatus, std::string &JobStr, std::string & DeviceStatus )
{
	CHtmlTable htmlJobsTable,htmlElapsedTable,htmlTable; 
	JobStatus.clear();
	JobStr.clear();
	DeviceStatus.clear();

	//Job* job = (Job *)  _cmsd->jobs->at(0).get(); // there may be multiple jobs - but only 1st counts in our world
	std::string  elapsedHeader="Date,Simulated<br>Time, Simulated<br>Seconds, Togo, Deadline";
	htmlElapsedTable.SetHeaderColumns( elapsedHeader);
 	std::string  jobElapsed=jobs->serviceTime.GetCurrentDateTime() + "," ;
	jobElapsed+=CTimestamp::ClockFormatofSeconds(jobs->TimeElapsed()) + ",";

	//jobElapsed += jobs->serviceTime.ElapsedString() + ",";
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
		jobHtml = jobs->parts[i] +",";
		jobHtml +=StdStringFormat("%6d,%6d", jobs->finishedparts[ jobs->parts[i]],jobs->totnumparts[jobs->parts[i]]); 
		htmlJobsTable.AddRow(jobsheader, jobHtml);
	}
	JobStatus =  htmlElapsedTable.CreateHtmlTable() + "<BR>" + htmlJobsTable.CreateHtmlTable()+"<BR>\n";

	//pHtmlView->SetElementId( "JobStatus", htmlElapsedTable.CreateHtmlTable() + "<BR>" + htmlJobsTable.CreateHtmlTable()+"<BR>\n");
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

	JobStr= htmlJobsTable.CreateHtmlTable()+"<BR>\n";
	//pHtmlView->SetElementId( "Jobs", htmlJobsTable.CreateHtmlTable()+"<BR>\n");
	////////////////////////////////////////////////////////////////
	std::string units = _wndMain->_bMinutes ? "Minutes" : "Seconds";
	double _timeDivisor = _wndMain->_bMinutes ? 60.0 : 1.0;
	std::string	header = "#,Machine,State,InQ,InQ<BR>Max,Current,MTP,TP<BR>Left," + Factory[0]->_statemachine->GenerateCSVHeader(units);
	htmlTable.SetHeaderColumns( header);

	for(int i=0;i<Factory.size() ; i++)
	{
		std::string html1=StdStringFormat("%d,",i);

		html1+= StdStringFormat("<A HREF=\"%s\">",(::ExeDirectory()+Factory[i]->_statemachine->Name()).c_str() );
		html1+=Factory[i]->_statemachine->Name() + "</A>,";

		//html1+=_resourceHandlers[i]->_statemachine->Name() + ",";
		html1+=StateStr(Factory[i]->_statemachine->GetState())  + "," ;
		html1+=StdStringFormat("%d", Factory[i]->_statemachine->size())  + "," ;
		html1+=StdStringFormat("%d", Factory[i]->_statemachine->MaxSize())  + "," ;
		if(Factory[i]->_statemachine->Current()!=NULL) 
			html1+="*,"; 
		else html1+="-,";  
		html1+=StdStringFormat("%8.4f", Factory[i]->_statemachine->MTTP)  + "," ;
		if(Factory[i]->_statemachine->Current()!=NULL)
			html1+=StdStringFormat("%8.4f", Factory[i]->_statemachine->Current()->_mttp)  + "," ;
		else html1+=" ,";
		
		html1+= Factory[i]->_statemachine->GenerateCSVTiming(_timeDivisor)  ;
		htmlTable.AddRow(header, html1);
	}

	DeviceStatus= htmlTable.CreateHtmlTable();
}


std::string Reporting::GenerateHtmlReport(CJobCommands * jobs,std::string filename)
{
	CHtmlTable htmlTable; 
	std::string html = CHtmlTable::CreateHtmlSytlesheetFrontEnd("Precision Sand Casting Saginaw MI");;
	//html += htmlTable.HtmlGoogleChart(states);
	html += htmlTable.HtmlRaphaeleChart( );

	//html+= htmlTable.HtmlRaphaeleChartData(states );


	html += "</HEAD>\n";

	//std::string html = CHtmlTable()::CreateHtmlDocument();
	std::string header = "Machine," + Factory[0]->_statemachine->GenerateCSVHeader("Minutes") + ",Utility<BR>Costs</BR>,Description";
	std::string alignment = ",right,right,right,right,right,right,right,";
	htmlTable.SetAlignment(alignment);
	htmlTable.SetHeaderColumns( header);

	for(int i=0;i<Factory.size() ; i++)
	{
		std::string html1=Factory[i]->_statemachine->Name() + ","; ///???
		html1+= Factory[i]->_statemachine->GenerateCSVTiming(60.00) + "," ;
		html1+= Factory[i]->_statemachine->GenerateTotalCosts("","");
		html1+=  (LPCSTR) Factory[i]->_resource->description;
		htmlTable.AddRow(header, html1);
	}

	CHtmlTable htmlRawTable; 
	std::string headerRaw = "Machine,Blocked,Starved,Down,Production,Off";
	htmlRawTable.SetHeaderColumns( headerRaw);
	std::map<std::string,double> rawstates;
	for(int i=0;i<Factory.size() ; i++)
	{
		std::string html3;
		Factory[i]->_statemachine->GenerateStateReport(rawstates,1.0 );
		html3+=Factory[i]->_statemachine->Name() + ",";
		html3+=StdStringFormat("%8.4f,%8.4f,%8.4f,%8.4f,%8.4f\n",
			rawstates["blocked"],rawstates["starved"],rawstates["down"],rawstates["production"],rawstates["off"]);
		htmlRawTable.AddRows(headerRaw, html3);
	}

	CHtmlTable kpiTable; 
	std::string kpiheader = "Machine,MTBF,MTTR,INQ, OUTQ";
	kpiTable.SetHeaderColumns( kpiheader);
	for(int i=0;i<Factory.size() ; i++)
	{
		std::string machine=Factory[i]->_statemachine->Name() + ",";
		machine+= StdStringFormat("%8.4f,",Factory[i]->_statemachine->MTBF  );
		machine+= StdStringFormat("%8.4f,",Factory[i]->_statemachine->MTTR  );
		machine+= StdStringFormat("%d,",Factory[i]->_statemachine->MaxSize()  );
		kpiTable.AddRows(kpiheader, machine);
	}

	CHtmlTable htmlTable2; 
	std::string header2 = "Machine,Units,Name,State,Time,Cost";
	htmlTable2.SetHeaderColumns( header2);
	for(int i=0;i<Factory.size() ; i++)
	{
		std::string machine=Factory[i]->_statemachine->Name() + ",";
		std::string html2= Factory[i]->_statemachine->GenerateCosts(machine,"\n") ;
		htmlTable2.AddRows(header2, html2);
	}

	html+=	"<h1>Precision Casting</h1>\n";
	for(int i=0; i < jobs->at(0)->_cmsd->documentation->size(); i++)
		html+= "<p> " + UrlDecode (   (LPCSTR) ((Documentation *) jobs->at(0)->_cmsd->documentation->at(i).get())->description);
	html+=	"<p>\n";

	html += htmlRawTable.CreateHtmlTable();
	html+=	"<p>\n";
	html += kpiTable.CreateHtmlTable();


	html+=	"<h2>Casting Line 1 Performance</h2>\n";

	for( std::map<std::string, int >::iterator it = jobs->finishedparts.begin(); it!= jobs->finishedparts.end(); it++)
		html+=StdStringFormat("<BR> Part %s Number Made = %d\n" , (*it).first.c_str(), (*it).second).c_str();
	html+=	StdStringFormat("<br>Time elapsed %8.4f in hours\n", jobs->_dUpdateRateSec/3600.0);
	html+=	StdStringFormat("<br>Time elapsed %8.4f in minutes\n", jobs->_dUpdateRateSec/60.0);
	html+=	StdStringFormat("<br>Time elapsed %8.4f in seconds\n", jobs->_dUpdateRateSec);
	
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
	for(int i=0;i<Factory.size() ; i++)
	{
		std::string machine=Factory[i]->_statemachine->Name() + ",";
		std::map<std::string,double> states;
		Factory[i]->_statemachine->GenerateStateReport(states,1000.0 );;
		Table.AppendTableCell(Raphael::InlinePieChart(states, machine + " State Use"));
	}
	html+=Table.MakeTable();

	html+= "</BODY>\n</HTML>\n";

	WriteFile(filename, html);
	return html;
}

