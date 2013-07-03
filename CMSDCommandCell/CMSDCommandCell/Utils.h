//
// Utils.h
//

#pragma once

#include <ctime>
#include <string>
#include "StdStringFcn.h"


class CTimestamp
{
public:
	CTimestamp() ;
	void Start() ;
	void Stop() ;
	unsigned long Elapsed() ;
	std::string ElapsedString() ;
	std::string StartTimeString() ;
	std::string EndTimeString() ;
	void StopWatch(int secondsFromNow);
	std::string ToGoTimeString();
	std::string GetDate() ;
	std::string GetCurrentDateTime() ;

private:
	std::time_t startTime;
	std::time_t endTime;
	unsigned long elapsed;

	std::string GetFormattedTime(std::time_t rawtime, std::string format="%m/%D/%Y %H:%M:%S");
	std::string GetTotalSeconds(std::time_t rawtime);

};

