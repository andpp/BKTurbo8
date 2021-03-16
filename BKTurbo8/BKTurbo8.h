#pragma once

enum OP_MODE
{
	UNKN,
	LINK,
	COMP,
	CL
};

int workCycle(std::string &strInFileName);
void Usage();
bool SkipTailString(wchar_t &ch);
void ParseLine();
void SaveFile(const std::string &strName);
void PrintLabelTable(const std::string &strName, const std::string &strExt);

