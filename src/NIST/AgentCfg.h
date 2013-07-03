//
// AgentCfg.h
//

// This software was developed by U.S. Government employees as part of
// their official duties and is not subject to copyright. No warranty implied 
// or intended.
#pragma once
#include <boost/thread.hpp>
#include <vector>
#include <string>

class AgentConfiguration;

class CAgentCfg
{
public:
	CAgentCfg(void);
	~CAgentCfg(void);
	void SetHttpPort(int portnum) { _agenthttpport=portnum; }
	void SetCfgfolder(std::string cfgfolder) { _cfgFolder=cfgfolder; }
	void SetCfgFile(std::string cfgfile) { _cfgFile=cfgfile; }
	void SetXmlFile(std::string xmlfile) { _xmlFile=xmlfile; }
	void AddDevice(std::string cfgname,  // M1, M2
		std::string devicefilename, // lookup name in devices.xml directory
		std::string ip, 
		std::string port);
	void UpdateDevice(std::string cfgname, 
		std::string ip="", 
		std::string port="");
	void Run();
	void SpawnProcess();
//void							StopHeartbeat() { _agentthread.interrupt(); }
	void Start(AgentConfiguration *config= NULL) 
	{
		_config=config;
		_agentthread= boost::thread(boost::bind(&CAgentCfg::Run, this)); 
	}
	LRESULT	WriteCfgFile(std::string cfgfile, std::string xmlfile,  std::string _destFolder);
	LRESULT	WriteAgentCfgFile(std::string cfgfile, std::string xmlfile, std::string _destFolder);
	LRESULT	WriteDevicesFile(std::string xmlfile,  std::string _destFolder);
	std::string ReadXmlDevicesFile(std::string name);
	void SetDevicesXmlFolder(std::string folder) { _cfgFolder= folder; }
	void SetDestinationFolder(std::string folder) { _destFolder= folder; }
	void Clear() ;
	void OnSaveCfg();
	std::string FindXmlDevicesFilePath(std::string device);

	std::vector<std::string>		ips, ports, rates, debugs, devices,names, alldevices;
	std::string 					_destFolder,_cfgFolder,_cfgFile,_xmlFile;
	std::string						_serviceName;
	boost::thread					_agentthread;
	int								_agenthttpport;
	AgentConfiguration * _config;

};