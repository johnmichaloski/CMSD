

#ifndef __INICONFIG_H__
#define __INICONFIG_H__

#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <vector>
#include "StdStringFcn.h"

#pragma comment( user, "Compiled on " __DATE__ " at " __TIME__ ) 


namespace crp
{
	/**
	* The StringVariant class is a simple string-based variant implementation that allows
	* the user to easily convert between simple numeric/string types.
	*/

	class StringVariant
	{
		std::string data;
	public:
		StringVariant() : data() {}

		operator LPCSTR() { return this->c_str(); }
		operator double() { return this->toNumber<double>(); }
		operator int() { return this->toNumber<int>(); }
		template<typename ValueType>
		StringVariant(ValueType val)
		{
			std::ostringstream stream;
			stream << val;
			data.assign(stream.str());
		}

		template<typename ValueType>
		StringVariant& operator=(const ValueType val)
		{
			std::ostringstream stream;
			stream << val;
			data.assign(stream.str());

			return *this;
		}

		template<typename NumberType>
		NumberType toNumber() const
		{
			NumberType result = 0;
			std::istringstream stream(data);

			if(stream >> result)
				return result;
			else if(data == "yes" || data == "true")
				return 1;

			return 0;
		}

		std::string str() const
		{
			return data;
		}
		
		const char * c_str() const
		{
			return data.c_str();
		}
	};


	/**
	* The Config class can be used to load simple key/value pairs from a file.
	*
	* @note An example of syntax:
	*	// An example of a comment
	*	username= Bob 
	*	gender= male 
	*	hair-color= black // inline comments are also allowed
	*	level= 42
	*
	* @note An example of usage:
	*	Config config;
	*	config.load("myFile.txt");
	*	
	*	std::string username = config["username"].str();
	*	int level = config["level"].toNumber<int>();
	*
	*	Config config;
	*	config.load(inifile);
	*	OutputDebugString(config.GetSymbolValue("FANUC.INCHES", L"INCHES").c_str());
	*	OutputDebugString(StrFormat("%f\n", config.GetSymbolValue("MAIN.MAXRPM", "9000").toNumber<double>()));

	*/

	class Config
	{
		typedef std::map<std::string, std::vector<std::string> > SectionMap;
		SectionMap sections;
			std::map<std::string, StringVariant> inimap;
		typedef std::map<std::string, StringVariant>::iterator ConfigIt;
public:
		Config() {}
		std::vector<std::string> GetSections() 
		{
			std::vector<std::string> s;
			for(std::map<std::string, std::vector<std::string> >::iterator it=sections.begin(); it!= sections.end(); it++)
			{
				s.push_back((*it).first);
			}
			return s;
		}

		// The actual map used to store key/value pairs

		// Utility function to trim whitespace off the ends of a string
		std::string trim(std::string source, std::string delims=" \t\r\n")
		{
			std::string result = source.erase(source.find_last_not_of(delims) + 1);
			return result.erase(0, result.find_first_not_of(delims));
		}
		// Loads key/value pairs from a file
		// Returns whether or not this operation was successful

		std::vector<std::string> getkeys(std::string section) { return sections[section]; }
		std::map<std::string, std::string> getmap(std::string section) 
		{ 
			std::map<std::string, std::string> mapping;
			if(sections.find(section) == sections.end())
				return mapping;
			std::vector<std::string> keys = sections[section];
			for(int i=0; i< keys.size(); i++)
			{ 
				mapping[keys[i]] = GetSymbolValue(section + "." + keys[i]);
			}
			return mapping; 
		}
		bool Config::load(const std::string filename)
		{
			std::string section;
			std::string line;
			std::string comment = "#";			
			std::string delimiter = "=";

			std::ifstream file(filename.c_str());

			if(!file.is_open())
				return false;

			while(!file.eof())
			{ 
				getline(file, line);

				// Remove any comments
				size_t commIdx = line.find(comment);
				if(commIdx != std::string::npos)
					line = line.substr(0, commIdx);

				// This should only match [section], not a=b[3]

				line=trim(line);
				size_t delimIdx = line.find("[");
				if( (line.find("[") == 0) && (line.rfind("]")==(line.size()-1)) )
				{
					line = trim(line);
					line = line.erase(line.find("]"));
					line = line.erase(0, line.find("[")+1);
					section=trim(line);
					continue;
				}

				
				delimIdx = line.find(delimiter);
				if(delimIdx == std::string::npos)
					continue;

				std::string key = trim(line.substr(0, delimIdx));
				std::string value = trim(line.substr(delimIdx + 1));
				sections[section].push_back(key);

				if(!key.empty())
				{
					if(!section.empty())
						key = section + "." + key;
					inimap[key] = value ;
				}
			}

			file.close();

			return true;
		}
		// Manually add section/key/value
		void AddKeyValue(std::string section, std::string key, std::string value)
		{
			sections[section].push_back(key);
			inimap[key] = value ;
		}
		void SetKeyValue(std::string section, std::string key, std::string value)
		{
			inimap[key] = value ;
		}
		// Use the [] operator to get/set values just like a map container
		const StringVariant& Config::operator[](const std::string& keyName) const 
		{
			std::map<std::string, StringVariant>::const_iterator iter = inimap.find(keyName);

			if(iter != inimap.end())
				return iter->second;

			static StringVariant empty;

			return empty;
		}
		StringVariant GetSymbolValue(std::string keyName, StringVariant szDefault=StringVariant())
		{

			std::map<std::string, StringVariant>::const_iterator iter = inimap.find(keyName);

			if(iter != inimap.end())
				return iter->second;
			return szDefault;

		}
		bool IsSymbol(std::string keyName)
		{

			std::map<std::string, StringVariant>::const_iterator iter = inimap.find(keyName);

			if(iter != inimap.end())
				return true;
			return false;

		}
	};
	inline std::vector<std::string> ParseFields(std::string fields)
	{
		std::vector<std::string> _fields = Tokenize(fields,",");
		for(int i=0; i< _fields.size(); i++)
		{
			if(_fields[i].empty())
			{
				_fields.erase(_fields.begin() + i);
				i--;
			}
			Trim(_fields[i]);
		}
		return _fields;
	}
};

#endif


