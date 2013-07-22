//
//
//

#pragma once

#include "CmdCell.h"
#include "ResourceHandler.h"

class Reporting
{
public:
	Reporting(void);
	~Reporting(void);
	static void AgentStatus(CJobCommands * jobs, std::string &JobStatus, std::string &Jobs, std::string & DeviceStatus );
	static std::string GenerateHtmlReport(CJobCommands * jobs, std::string filename);
	static std::string CreateHistory(double dPixelWidth, std::vector<int> _historian, std::vector<double> _timing) ;
	static std::string GenerateResourceReport(int i);

};

