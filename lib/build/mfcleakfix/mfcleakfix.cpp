
// Adapted from:
// http://www.eggheadcafe.com/software/aspnet/32623009/why-does-mfc-call-crtdum.aspx

#include <crtdbg.h>
#include <string.h>

#include "mfcleakfix.h"

// Dummy function to make sure the DLL is loaded:
IGNORE_MFC_LEAKS_API int use_ignore_mfc_leaks(void)
{
	return 0;
}

static _CRT_REPORT_HOOK prevHook;
static bool SwallowReport;

class MemoryLeakDetector
{
public:
	MemoryLeakDetector();
	virtual ~MemoryLeakDetector();
};

int ReportingHook( int reportType, char* userMessage, int* retVal );

MemoryLeakDetector::MemoryLeakDetector()
{
	//don't swallow assert and trace reports
	SwallowReport = false;
	//change the report function
	prevHook = _CrtSetReportHook(ReportingHook);
}

//this destructor is called after mfc has died
MemoryLeakDetector::~MemoryLeakDetector()
{
	//make sure that there is memory leak detection at the end of the program
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)	|
		_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF	| _CRTDBG_CHECK_ALWAYS_DF);

	//reset the report function to the old one
	_CrtSetReportHook(prevHook);
}

static MemoryLeakDetector MLD;  //this lives as long as this translation unit

int ReportingHook( int reportType, char* userMessage, int* retVal )
{
	//_CrtDumpMemoryLeaks() outputs "Detected memory leaks!\n" and calls
	//_CrtDumpAllObjectsSince(NULL) which outputs all leaked objects,
	//ending this (possibly long) list with "Object dump complete.\n"
	//In between those two headings I want to swallow the report.

	if ((strcmp(userMessage,"Detected memory leaks!\n") == 0) || SwallowReport) {
		if (strcmp(userMessage,"Object dump complete.\n") == 0)
			SwallowReport = false;
		else
			SwallowReport = true;
		return true;  //swallow it
	}
	else
	return false; //give it back to _CrtDbgReport()
};

