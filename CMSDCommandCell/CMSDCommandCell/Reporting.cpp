#include "StdAfx.h"
#include "Reporting.h"
#include "HtmlTable.h"
#include "ResourceHandler.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include "KPI.h"

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
	jobsheader = "JobId,PartId, Now, Ops, Machine,Max Steps,Order Time, Factory Time" ;
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
		jobHtml +=CTimestamp::ClockFormatofSeconds(jobs->at(i)->orderTime.SimElapsed()) + ",";
		jobHtml +=CTimestamp::ClockFormatofSeconds(jobs->at(i)->factoryTime.SimElapsed())  ;
		htmlJobsTable.AddRow(jobsheader, jobHtml);
	}

	JobStr= htmlJobsTable.CreateHtmlTable()+"<BR>\n";
	//pHtmlView->SetElementId( "Jobs", htmlJobsTable.CreateHtmlTable()+"<BR>\n");
	////////////////////////////////////////////////////////////////
	std::string units = _wndMain->_bMinutes ? "Minutes" : "Seconds";
	double _timeDivisor = _wndMain->_bMinutes ? 60.0 : 1.0;
	std::string	header = "#,Machine,State,InQ,InQ<BR>Max,At,MTP,TP<BR>Left," + Factory[0]->_statemachine->GenerateCSVHeader(units) + ",Utilites,Description";
	std::string	alignment = "center,left,center,right,right,right,right,right,right,right,right,right,right,right,right";
	htmlTable.SetHeaderColumns(header);
	htmlTable.SetAlignment(alignment);

	for(int i=0;i<Factory.size() ; i++)
	{
		std::string html1=StdStringFormat("%d,",i);

		html1+= StdStringFormat("<A HREF=\"%s\">", ("http://"+Factory[i]->_statemachine->Name()).c_str() );
		html1+=Factory[i]->_statemachine->Name() + "</A>,";

		//html1+=_resourceHandlers[i]->_statemachine->Name() + ",";
		html1+=Factory[i]->_statemachine->GetStateName()  + "," ;
		html1+=StdStringFormat("%d", Factory[i]->_statemachine->size())  + "," ;
		html1+=StdStringFormat("%d", Factory[i]->_statemachine->MaxSize())  + "," ;
		if(Factory[i]->_statemachine->Current()!=NULL) 
			html1+="*,"; 
		else html1+="-,";  
		html1+=StdStringFormat("%8.2f", Factory[i]->_statemachine->MTTP)  + "," ;
		if(Factory[i]->_statemachine->Current()!=NULL)
			html1+=StdStringFormat("%8.2f", Factory[i]->_statemachine->Current()->_mttp)  + "," ;
		else html1+=" ,";
		
		html1+= Factory[i]->_statemachine->GenerateCSVTiming(_timeDivisor)   + "," ;
		
		html1+= Factory[i]->_statemachine->GenerateTotalCosts("","");// Fixme: automatically appends ,
		html1+=  (LPCSTR) Factory[i]->_resource->description;

		htmlTable.AddRow(header, html1);
	}

	DeviceStatus= htmlTable.CreateHtmlTable();
}


std::string Reporting::GenerateHtmlReport(CJobCommands * jobs,std::string filename)
{
	CHtmlTable htmlTable; 
	std::string html = CHtmlTable::CreateHtmlSytlesheetFrontEnd("Precision Sand Casting Saginaw MI");;
	//html += htmlTable.HtmlGoogleChart(states);
	html += Raphael::HtmlRaphaeleChart( );

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
	std::string headerRaw = "Machine,Down,Blocked,Starved,Production,Off";
	htmlRawTable.SetHeaderColumns( headerRaw);
	std::map<std::string,double> rawstates;
	for(int i=0;i<Factory.size() ; i++)
	{
		std::string html3;
		Factory[i]->_statemachine->GenerateStateReport(rawstates,1.0 );
		html3+=Factory[i]->_statemachine->Name() + ",";
		html3+=StdStringFormat("%8.2f,%8.2f,%8.2f,%8.2f,%8.2f\n",
			rawstates["down"], rawstates["blocked"],rawstates["starved"],rawstates["production"],rawstates["off"]);
		htmlRawTable.AddRows(headerRaw, html3);
	}

	CHtmlTable kpiTable; 
	std::string kpiheader = "Machine,MTBF,MTTR,INQ";
	kpiTable.SetHeaderColumns( kpiheader);
	for(int i=0;i<Factory.size() ; i++)
	{
		std::string machine=Factory[i]->_statemachine->Name() + ",";
		machine+= StdStringFormat("%8.2f,",Factory[i]->_statemachine->MTBF  );
		machine+= StdStringFormat("%8.2f,",Factory[i]->_statemachine->MTTR  );
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
	//for(int i=0; i < jobs->at(0)->_cmsd->documentation->size(); i++)
	//	html+= "<p> " + UrlDecode (   (LPCSTR) ((Documentation *) jobs->at(0)->_cmsd->documentation->at(i).get())->description);
	//html+=	"<p>\n";

	html += htmlRawTable.CreateHtmlTable();
	html+=	"<p>\n";
	html += kpiTable.CreateHtmlTable();


	html+=	"<h2>Casting Line 1 Performance</h2>\n";

	for( std::map<std::string, int >::iterator it = jobs->finishedparts.begin(); it!= jobs->finishedparts.end(); it++)
		html+=StdStringFormat("<BR> Part %s Number Made = %d\n" , (*it).first.c_str(), (*it).second).c_str();
	html+=	StdStringFormat("<br>Time elapsed %8.2f in hours\n", jobs->_dUpdateRateSec/3600.0);
	html+=	StdStringFormat("<br>Time elapsed %8.2f in minutes\n", jobs->_dUpdateRateSec/60.0);
	html+=	StdStringFormat("<br>Time elapsed %8.2f in seconds\n", jobs->_dUpdateRateSec);
	
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
		std::map<std::string,double> states;
		std::string machine=Factory[i]->_statemachine->Name() + ",";
		Factory[i]->_statemachine->GenerateStateReport(states,1000.0 );
		std::vector<double> values; std::vector<std::string> names;
		for(std::map<std::string,double>::iterator it = states.begin(); it!=states.end(); it++)
		{
			values.push_back((*it).second) ;
			names.push_back((*it).first) ;
		}

		// transform(states.begin(), states.end(), back_inserter(names), bind( &std::map<std::string,double>::value_type::first, _1));
		
		Table.AppendTableCell(Raphael::InlinePieChart(values, names, machine + " State Use"));
	}
	html+=Table.MakeTable();

	html+= "</BODY>\n</HTML>\n";

	WriteFile(filename, html);
	return html;
}


/**
	if(_historian.size() == 0)
	{
		_historian.push_back(GetState());
		_historiantimestamp.push_back(GetTimeStamp(LOCAL));
		statetimer.restart();
	}

		while(seconds > nSampleTimeSecs)
	{
		std::string msg = GetTimeStamp(LOCAL);
		_historiantimestamp.push_back(msg);
		_historian.push_back(_laststate);
		seconds-=nSampleTimeSecs;
	}
	if(_historian.size() >= (UINT) wpx )
	{
		_historian.erase(_historian.begin()); // erase beginning entry
		_historiantimestamp.erase(_historiantimestamp.begin());
	}

	*/
std::string Reporting::CreateHistory(double dPixelWidth,	std::vector<int> _historian, std::vector<double> _timing) 
{ 
	std::string html;
	dPixelWidth=600.0;

	// History Table insertion
	html+= StdStringFormat("<TD cellpadding=0 cellspacing=0 border=0 width=%dpx>\n", (long) dPixelWidth);
	html+= "<table id=\"myTable\" class=\"histogram\" style='empty-cells: show; cellpadding=0; cellspacing=0; border=0;'>";
	html+= "<tr>\n";
	double total=std::accumulate(_timing.begin(),_timing.end(),0.0, std::plus<double>());

	char  * colormap[5] = { "red", "blue", "green", "yellow", "black"};
	for(int i=0; i < _historian.size() ; i++)
	{
			html+="<td style='background-color:" + std::string(colormap[_historian[i]])  + "' ";
//			html+="title=\"" + _historiantimestamp[i] + "\"";
			html+=">";
			html+=StdStringFormat("	<DIV style='width:%dpx;'/></td>", (int) (_timing[i]/total * dPixelWidth));
	}


	html+= "</tr>\n";
	html+= " </table>	\n";		
	html+= "<TD/>\n";
	html+= "\n";
	return html;
}
static int StrToInt(std::string const& s)
{
//	char  * colormap[5] = { "red", "blue", "green", "yellow", "black"};

	if(s=="off") return 4;
	if(s=="ready") return 4;
 	if(s=="starved") return 3;
 	if(s=="blocked") return 1;
 	if(s=="faulted") return 0;
  	if(s=="running") return 2;

	return 4;
}

std::string Reporting::GenerateResourceReport(int i)
{

	std::string html;
	std::string filename = ::ExeDirectory()+Factory[i]->_statemachine->Name() + ".html";
	//boost::circular_buffer<EventType> history( Factory[i]->_statemachine->eventtrace);

	html+="<html>\n";	
	html += CHtmlTable::CreateHtmlFrontEnd(Factory[i]->_statemachine->Name());
	html += Raphael::HtmlRaphaeleChart( );

	//html+= htmlTable.HtmlRaphaeleChartData(states );


	html += "</HEAD>\n";
	html += "<body>\n";
	html += StdStringFormat("<H1>%s</h1>\n", Factory[i]->_statemachine->Name().c_str());

	CHtmlTable kpiTable; 
	std::string kpiheader = "Machine,MTTP, MTBF,MTTR,INQ";
	kpiTable.SetHeaderColumns( kpiheader);
	std::string alignment = "right,right,right,right,right,";
	kpiTable.SetAlignment(alignment);

	std::string machine=Factory[i]->_statemachine->Name() + ",";
	machine+= StdStringFormat("%8.2f,",Factory[i]->_statemachine->MTTP  );
	machine+= StdStringFormat("%8.2f,",Factory[i]->_statemachine->MTBF  );
	machine+= StdStringFormat("%8.2f,",Factory[i]->_statemachine->MTTR  );
	machine+= StdStringFormat("%d",Factory[i]->_statemachine->MaxSize()  );
	kpiTable.AddRows(kpiheader, machine);
	html += kpiTable.CreateHtmlTable();
	
	std::vector<int> history;
	for( int j = 0; j< Factory[i]->_statemachine->statetrace.size() ; j++ )
		history.push_back( StrToInt( Factory[i]->_statemachine->statetrace[j]) );


	html+="<BR><TABLE class=\"gridtable\"><TR> <TH> Machine</TH><TH> Histogram</TH></TR>";
	html+="<TR><TD>" + Factory[i]->_statemachine->Name() + "</TD>";
	html+= CreateHistory(800.0, history, Factory[i]->_statemachine->timetrace);
	html += "</TR></TABLE><BR>" ;

//	Factory[i]->_statemachine->stats.nTotalParts= _wndMain->jobs->partStats.nTotalParts;
//	Factory[i]->_statemachine->stats.nGoodParts= _wndMain->jobs->partStats.nGoodParts;

	KPI kpi(Factory[i]->_statemachine->stats);
	CHtmlTable kpiHtmlTable; 
	kpiHtmlTable.SetHeaderColumns( "Value,Abbrev, KPI, Equation, Description");
	//kpiHtmlTable.AddRows("Value,Abbrev, KPI, Equation", kpi.AbbrvCSVString());
	kpiHtmlTable.AddRows("Value,Abbrev, KPI, Equation, Description", kpi.ByResources(Factory[i]->_statemachine->stats),"@");
	html += kpiHtmlTable.CreateHtmlTable();
	
	
	html+="</body></html>\n";
	WriteFile(filename, html);
	return html;
}


//std::string Reporting::HtmlRaphaelLineData(std::vector < std::vector<double>  > &data )
//{
//	std::string tmp;
//
//	tmp +="	 <script>\n";
//	tmp +="	         window.onload = function () {\n";
//	tmp +="	             var r = Raphael(\"holder\"),\n";
//	tmp +="	                pie = r.piechart(320, 240, 100, [";
//	for(std::map<std::string,double>::iterator it = states.begin(); it!=states.end(); it++)
//		tmp+=StdStringFormat("%8.2f,", (*it).second) ;
//
//	tmp=tmp.substr(0,tmp.size()-1); // skip last ,
//	tmp +=" ],\n";
//
//	tmp +="               { legend: [";
//
//	for(std::map<std::string,double>::iterator it = states.begin(); it!=states.end(); it++)
//		tmp+="\"%%.%% - " +  (*it).first + "\"," ;
//	tmp=tmp.substr(0,tmp.size()-1); // skip last ,
//
//	tmp +="],";
//
//	tmp +="	legendpos: \"west\", href: [\"http://raphaeljs.com\", \"http://g.raphaeljs.com\"]});\n";
//
//	tmp +="	            r.text(320, 100, \"Cell Resource State Use\").attr({ font: \"20px sans-serif\" });\n";
//	tmp +="	            pie.hover(function () {\n";
//	tmp +="	                this.sector.stop();\n";
//	tmp +="	                this.sector.scale(1.1, 1.1, this.cx, this.cy);\n";
//
//	tmp +="	                if (this.label) {\n";
//	tmp +="	                    this.label[0].stop();\n";
//	tmp +="	                    this.label[0].attr({ r: 7.5 });\n";
//	tmp +="	                    this.label[1].attr({ \"font-weight\": 800 });\n";
//	tmp +="	                 }\n";
//	tmp +="	             }, function () {\n";
//	tmp +="	                this.sector.animate({ transform: 's1 1 ' + this.cx + ' ' + this.cy }, 500, \"bounce\");\n";
//
//	tmp +="	                if (this.label) {\n";
//	tmp +="	                   this.label[0].animate({ r: 5 }, 500, \"bounce\");\n";
//	tmp +="	                   this.label[1].attr({ \"font-weight\": 400 });\n";
//	tmp +="	               }\n";
//	tmp +="	           });\n";
//	tmp +="	       };\n";
//	tmp +="	   </script>\n";
//
//
//	return tmp;
//}
