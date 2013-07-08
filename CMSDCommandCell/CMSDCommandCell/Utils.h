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
	static std::string GetFormattedTime(std::time_t rawtime, std::string format="%m/%D/%Y %H:%M:%S");
	static std::string ClockFormatofSeconds(double seconds)
	{
		int time = seconds;
		int hour=time/3600;
		time=time%3600;
		int min=time/60;
		time=time%60;
		int sec=time;
		return StdStringFormat("%02d:%02d:%02d", hour, min, sec);
	}
private:
	std::time_t startTime;
	std::time_t endTime;
	unsigned long elapsed;

	std::string GetTotalSeconds(std::time_t rawtime);

};

