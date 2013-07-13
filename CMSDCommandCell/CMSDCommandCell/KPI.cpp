#include "StdAfx.h"
#include "KPI.h"
#include "HtmlTable.h"

KPI::KPI(Stats stats)
{
	Init(stats);
}

void KPI::Init(Stats & stats1) 
{
	this->nBlockedTime=stats1.nBlockedTime;
	this->nStarvedTime=stats1.nStarvedTime;
	this->nDownTime=stats1.nDownTime;
	this->nProductionTime=stats1.nProductionTime;
	this->nOffTime=stats1.nOffTime;
	this->nRepairTime=stats1.nRepairTime;
	this->nIdleTime=stats1.nIdleTime;
	 nTotalParts=stats1.nTotalParts;
	nGoodParts=stats1.nGoodParts;

}

KPI::~KPI(void)
{
}
//void 
//OT
//PDT
//PCT
//BT
//PBT

void KPI::Compute() // throws error
{
	if(nTotalParts<=0)
		throw std::exception("Need parts to compute KPI \n");

	// (PTU The production time per unit is the scheduled time for producing one unit.

	// Line info - planned
	//tmp+="POT, Planned Operation Time =PBT + Planned Down time\n";
	PSUT=0.0; //Planned set up time (PRZ)\n";
	PBZ=PBT= AUPT + ADOT; //Planned Busy Time (PBZ) 
	PTU= 0.0; // Production time per unit (PEZ)\n";


	// Line info - actual

	ASUT=0.0; // actual setup time
	AUPT=APT= nProductionTime;  ///Actual production time 
	ADET=nBlockedTime+ nStarvedTime; // Actual unit delay time
	BT=APT + ADET + ASUT; //Busy time (BLZ)\n";
	SU= nStarvedTime; //, Delay time (SU)\n";
	SZ=nDownTime; //Down time Do (SZ)\n";
	ESUT=0; // Effective setup time == Real set up time / effective set up time (TRZ)\n";
	//OEE  = Availability * Effectiveness * Quality rate 
	PQ = nTotalParts; // produced quantity\n";
	SQ=0.0; // , Scrap Quantity\n";
	POQ=0; // FIXME: job total parts - // Production order quantity
	IP = 0; // Inspected Part (IP)\n";
	if(IP!= 0) 
		FPY = GP/IP; // Yield(FPY) =GP/IP\n";
	
	GP=nGoodParts; // Good pieces (GT) good quantity (GM)\n";

	// Unit operation (not resources) defintions
	//tmp+="OP, Operation process (AG)\n";
	//tmp+="OS, Operation sequence (AFO)\n";
	//tmp+="OT, Operation time (BZ)\n";

	AUP= (APT + ASUT) / nTotalParts; //  or AUP Actual unit processing time = APT + ASUT (Divided by parts?)
	AUPT = AUP / nTotalParts;
	AUBT= AUPT + ADET; //  Actual unit busy time (Divided by parts?)
	ADOT=nDownTime; //  Actual unit down time (Divided by parts?)
	AOET= dTotalTime; //  Actual order execution time
//	tmp+="PU, Production unit (PE)\n";


	// inventory
	//tmp+="CI, Consumables inventory\n";
	//tmp+="FGI, Finished goods inventory\n";

	//tmp+="LLV, Lower limit value (UGW)\n";
	//tmp+="LT, Labor time (PZt)\n";
	//tmp+="PCT, Process time (BAZ)\n";
	//tmp+="PDT, Production time (HNZ)\n";
	//tmp+="PO, Production order (FA)\n";
	//tmp+="POQ, Production order quantity (FAM)\n";
	//tmp+="PRU, PlannedRunTimePerUnit \n";
	//tmp+="PSQ, Planned scrap quantity (GAM)\n";
	//tmp+="RMI, Raw material inventory\n";
	//tmp+="RQ, Rework quantity (NM)\n";
	//tmp+="s, Standard deviation\n";
	//tmp+="TAT, Total attendance time (GAZ)\n";
	//tmp+="TPT, Lead time / throughput time (DLZ)\n";
	//tmp+="TT, Transportation time (TZ)\n";
	//tmp+="ULV, Upper limit value (OGW)\n";
	//tmp+="WG, Working group (AGR)\n";
	//tmp+="WIP, Work in process inventory (WPI)\n";
	//tmp+="WOT, Working time (PAZ)\n";
	//tmp+="WP, Work place (AP)\n";
	//tmp+="WT, Wait time (LZ)\n";

}


std::string KPI::AbbrvCSVString()
{
	std::string tmp;

	tmp+="ADET, Actual unit delay time\n";
	tmp+="ADOT, Actual unit down time\n";
	tmp+="AOET, Actual order execution time\n";
	tmp+="APT, Actual production time \n";
	tmp+="ASUT, Actual set up time \n";
	tmp+="AUBT, Actual unit busy time = AUPT + ADET\n";
	tmp+="AUPT, Actual unit processing time = APT + ASUT\n";
	tmp+="BT, Busy time (BLZ)\n";
	tmp+="CI, Consumables inventory\n";
	tmp+="DeT, Delay time (SU)\n";
	tmp+="DoT, Down time (SZ)\n";
	tmp+="ESUT, Real set up time / effective set up time (TRZ)\n";
	tmp+="FGI, Finished goods inventory\n";
	tmp+="FPY, Yield(FPY) =GP/IP\n";
	tmp+="GP, Good pieces (GT)\n";
	tmp+="GP, GoodParts \n";
	tmp+="GQ, Good quantity (GM)\n";
	tmp+="IP, Inspected Part (IP)\n";
	tmp+="IP, Inspected parts (PT)\n";
	tmp+="LLV, Lower limit value (UGW)\n";
	tmp+="LT, Labor time (PZt)\n";
	tmp+="MA, Machine\n";
	tmp+="OEE, OEE index = Availability * Effectiveness * Quality rate \n";
	tmp+="OP, Operation process (AG)\n";
	tmp+="OS, Operation sequence (AFO)\n";
	tmp+="OT, Operation time (BZ)\n";
	tmp+="PBT, Planned Busy Time = AUPT + ADOT \n";
	tmp+="PBT, Planned busy time (PBZ)\n";
	tmp+="PCT, Process time (BAZ)\n";
	tmp+="PDT, Production time (HNZ)\n";
	tmp+="PO, Production order (FA)\n";
	tmp+="POQ, Production order quantity (FAM)\n";
	tmp+="POT, Planned Operation Time =PBT + Planned Down time\n";
	tmp+="PQ, produced quantity\n";
	tmp+="PRU, PlannedRunTimePerUnit \n";
	tmp+="PSQ, Planned scrap quantity (GAM)\n";
	tmp+="PSUT, Planned set up time (PRZ)\n";
	tmp+="PTU, Production time per unit (PEZ)\n";
	tmp+="PU, Production unit (PE)\n";
	tmp+="RMI, Raw material inventory\n";
	tmp+="RQ, Rework quantity (NM)\n";
	tmp+="s, Standard deviation\n";
	tmp+="SQ, Scrap quantity (AM)\n";
	tmp+="SQ, Scrap Quantity\n";
	tmp+="TAT, Total attendance time (GAZ)\n";
	tmp+="TPT, Lead time / throughput time (DLZ)\n";
	tmp+="TT, Transportation time (TZ)\n";
	tmp+="ULV, Upper limit value (OGW)\n";
	tmp+="WG, Working group (AGR)\n";
	tmp+="WIP, Work in process inventory (WPI)\n";
	tmp+="WOT, Working time (PAZ)\n";
	tmp+="WP, Work place (AP)\n";
	tmp+="WT, Wait time (LZ)\n";



	tmp+=",Allocation ratio = AUBT / AOET\n";
	tmp+=",Production process ratio = APT / AOET\n";
	tmp+=",Setup rate = ASUT / AUPT\n";
	tmp+=" ,Effectiveness	= PRU * PQ / APT\n";
	tmp+=",Availability	= AUBT/ PBT\n";


	return tmp;
}
std::string KPI::CalculationCSVString()
{
	std::string tmp;

	tmp+="Allocation degree, 	,Allocation degree = BT/TPT	";
	tmp+="Allocation efficiency, 	Allocation efficiency = BT/PBT\n";		
	tmp+="Availability, 	= PDT/PBT	\n";	
	tmp+="Comprehensive Energy Consumption,	e, e = E/PQ =（∑Mi*Ri + Q）/ PQ\n";	

	tmp+="Critical Machine Capability Index, (Cmk),	Cmko = (ULV - xqq) / (3 * s) ; Cmku = (Xqq - LLV) / (3 * s)	\n";	
	tmp+="Critical Process Capability Index, (Cpk),	Cpko = (ULV - xqq) / (3 *  ) ;   Cpku = (xqq - LLV) / (3 *  )\n";	

	tmp+="Effectiveness,	,Effectiveness = PTU * PQ / PDT\n";	
	tmp+="Efficiency, 	,Efficiency = PDT/BT\n";	
	tmp+="Emission ratio,	,Emission ratio = (CO2energy + CO2transported goods + CO2travel + CO2internal) / VA	\n";
	tmp+="Energy ratio, 	,Energy ratio = (energy bought + energy internally produced) / VA\n";	
	tmp+="Equipment Load Rate,	,Equipment Load Rate = PQ / maximum equipment production capacity	\n";
	tmp+="Fall off Rate	, ,Fall off Rate = SQ / PQ of the first manufacturing step	\n";
	tmp+="Finished Goods Rate,	,Finished Goods Rate = GQ / consumed material\n";	
	tmp+="First Pass Yield, (FPY),	FPY = GP / PT\n";	
	tmp+="Harmful substances,	,Harmful substances = total used amount of harmful substances in tons / VA\n";	
	tmp+="Hazardous waste,	,Ratio of hazardous waste= total amount of hazardous waste/ VA\n";	
	tmp+="Integrated Goods Rate,	,Finished Goods Rate =  (Integrated Good quantity )/ (consumed material)\n";	
	tmp+="Inventory turns,	,Inventory Turns = Throughput / average inventory\n";	
	tmp+="Machine Capability Index, (Cm),	Cm = (ULV - LLV ) / (6 * s)\n";	
	tmp+="NEE Index, 	,NEE Index = PCT/PBT * Effectiveness * Quality rate\n";	
	tmp+="OEE Index,	,OEE Index = Availability * Effectiveness * Quality rate	\n";
	tmp+="Other Lost Rate,	,Other Lost Rate = other lost / consumed material	\n";
	tmp+="Preparation degree, 	,Preparation Degree = ESUT/BRZ	\n";
	tmp+="Process Capability Index, (Cp),	Cp = (ULV - LLV) / (6 * Sigma )\n";

	tmp+="Production Lost Rate,	,Production Lost Rate = production lost / consumed material	\n";
	tmp+="Quality Rate,	,Quality Rate = GQ / PQ	\n";
	tmp+="Ratio of used material,	,Ratio of used material = total amount of material used / VA	\n";
	tmp+="Reworking Ratio,	,Reworking Ratio = RQ / PQ	\n";
	tmp+="Storage and Transportation Lost Rate,	,Storage and Transportation Lost Rate = (storage and transportation )/(lost consumed material)	\n";
	tmp+="Technical Usage Level,	,Technical Usage Level = PDT / (PDT + DeT)\n";	
	tmp+="Throughput,	,Throughput = PQ/TPT\n";

	tmp+="Wastage Degree, 	, Wastage Degree = SQ / PSQ	<BR> Wastage Ratio, 	Wastage Ratio = SQ / PQ	\n";
	tmp+="Worker productivity, 	,Worker Productivity = WOT/TAT	\n";

	return tmp;
}

//OEE Calculation	OEE index = Availability * Effectiveness * Quality rate	
//Availability	= (Actual production time) / ( Planned Busy Time)
//= MTCpower == ON / ShiftDuration
//Effectiveness	= PRU * PQ / APT
//= PlannedRunTimePerUnit * ProducedQuantity / ActualProductionTime
//= MTCCycleTime * MTCPartCount / MTCpower == ON
//Quality Rate	= GoodQuantity / ProductionQuantity
//= MTCPartCountGood /MTCPartCountGood
//Planned Operation Time 
//	MTCmode != PreventiveMaintenance?
//Planned Down Time 	Capacity Multiplier for processing times e.g., 1.1 for 90% capacity 	
//Scrap Quantity (SQ)	MTCPartCountBad
//GoodParts (GP)	MTCPartCountGood
//Inspected Part (IP)	MTCPartCount (until we add sequencing to CMM)
//Yield(FPY)	GP/IP
//Produced Quantity (PQ)	MTCPartCount


std::string KPI::PlannedPeriods()
{
	std::string tmp;
	tmp+= "POT,  Order duration, ";
	tmp+= "The order duration is the scheduled time for executing a production task based on the work plan data. It is ";
	tmp+= "calculated from the production time per unit multiplied by the order quantity plus the planned set-up time.,";
	tmp+= "POT=OT+PSUT\n";
	
	tmp+= "OT,Operation Time, ";
	tmp+= "The operation time is that time in which a production unit can be used operational and personnel for ";
	tmp+= "production and maintenance. The operation time is a scheduled time.,";
	tmp+= "OT=QuantityProducts*(Resource/TimePerUnit)+ Maintenance\n";
	
	tmp+= "PSUT, Planned set-up time, ";
	tmp+= "The planned set-up time is the scheduled time for the set-up of a production unit for an order.,";
	tmp+= "PUST=0.0\n";

	tmp+= "PBT,Planned allocation time, ";
	tmp+= "The planned allocation time is the operating time minus the planned standstill (planned downtime). The ";
	tmp+= "planned standstill may be used for planned maintenance work. The planned allocation period is available for ";
	tmp+= "the detailed planning of machine usage with production orders.,";
	tmp+= "PBT=OT-PlannedStandstill\n";
	
	tmp+= "PTU, Production time per unit, ";
	tmp+= "The production time per unit is the scheduled time for producing one unit.,";
	tmp+="PTU=MTTP_partid\n";

	tmp+= ", Planned standstill, ";
	tmp+= "The planned standstill is the planned downtime,";
	tmp+="PlannedStandstill=0.0\n";

	return tmp;
}

std::string KPI::ReportingPlanned()
{
	CHtmlTable htmlPlannedTable;
	std::string htmlPlannedTableHeader="Abbrev,Term, Description, Formula";
	htmlPlannedTable.SetHeaderColumns(htmlPlannedTableHeader);
	htmlPlannedTable.AddRows(htmlPlannedTableHeader, PlannedPeriods());
	return htmlPlannedTable.CreateHtmlTable();
}

std::string KPI::Reporting(Stats &factoryStats)
{	
	CHtmlTable htmlStatsPlannedTable;
	std::string htmlStatsPlannedTableHeader="Machine, OT, PSUT, PBT, PTU ";
	htmlStatsPlannedTable.SetHeaderColumns(htmlStatsPlannedTableHeader);
	std::string html = factoryStats.Property("name") + ",";
	html +=  ConvertToString(factoryStats["OT"]) + ",";
	html +=  ConvertToString(factoryStats["PSUT"]) + ",";
	html +=  ConvertToString(factoryStats["PBT"]) + ",";
	html +=  ConvertToString(factoryStats["PTU"]) ;
	htmlStatsPlannedTable.AddRow(htmlStatsPlannedTableHeader, html);

	return htmlStatsPlannedTable.CreateHtmlTable();
}
std::string ActualPeriods()
{
	std::string tmp;

	tmp+= "WOT, Work time ,";
	tmp+= "The work time is the time that a production worker needs for the execution of a manufacturing order.";
	
	tmp+= "PCT, Processing time ,";
	tmp+= "The processing time is the time needed for set-up and for the main usage.";

	tmp+= "BT, Busy time ,";
	tmp+= "The busy time is the time that a production unit is used for the execution of a manufacturing order.";
	
	tmp+= "TPT, Execution time {TD},";
	tmp+= "The execution time is the time difference between start time and end time of a manufacturing order. It includes the busy time as well as the drop and transportation time.";
	
	tmp+= "TAT, Total attendance time,";
	tmp+= "The total attendance is the time that a production employee is present for working in the company. It is the difference between Come and Go but without breaks.";
	
	tmp+= "PDT, Main usage time {th},";
	tmp+= "The main usage time is the producing time of the machine. It includes only the value-adding functions.";
	
	tmp+= "LZ, Set aside time,";
	tmp+= "The set aside time is the time in which the material is not in progress of the manufacturing process and also is not on transport.";
	
	tmp+= "PZ, Staff time ,";
	tmp+= "The staff time is the total time consumed by the production staff to execute a manufacturing order.";
	
	tmp+= "DOT, Downtime,";
	tmp+= "The down time is the time when the machine is not running with orders, although they are available.";
	
	tmp+= "SU, Malfunction-caused interrupts {TBS, TMS},";
	tmp+= "The malfunction-caused interrupts are times, which occur unplanned during the order processing and thus lead to an unwanted extension of busy times.";
	
	tmp+= "ESUT, Actual set-up time,";
	tmp+= "The actual set-up time is the time that was consumed for the preparation of an order at a production unit.";
	
	tmp+= "TT, Transport time ,";
	tmp+= "The transport time is the time required for transport between production units, or to and from the warehouse of the used material.";

	return tmp;
}
std::string LogisticalQuantities()
{
	std::string tmp;

	tmp+= "POQ, Order quantity,";
	tmp+= "The order quantity is the planned quantity for a production order (lot size, production order quantity).";
	
	tmp+= "SQ, Wastage quantity,";
	tmp+= "The wastage quantity is the produced quantity that did not meet quality requirements and either has to be scrapped or recycled.";
	
	tmp+= "PSQ, Planned wastage quantity,";
	tmp+= "The planned wastage quantity is the amount of process-related wastage that is expected when manufacturing the product (e.g. at the start or ramp-up phases of the manufacturing systems).";
	
	tmp+= "GQ, Good quantity,";
	tmp+= "The good quantity is the produced quantity that meets quality requirements (see also 6.7.1).";
	
	tmp+= "RQ, Rework quantity,";
	tmp+= "The rework quantity is the produced quantity that missed the quality requirements. However, these requirements can be met by subsequent work.";
	
	tmp+= "PQ, Produced quantity,";
	tmp+= "The produced quantity is the quantity that a production unit has produced in relation to a production order.";

	return tmp;
}