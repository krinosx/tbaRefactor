#include <stdio.h>
#include "CuTest.h"


/** Function prototypes for test suites */
CuSuite* CommGetSuite(void);
CuSuite* dg_eventGetSuite(void);


void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, CommGetSuite());
	CuSuiteAddSuite(suite, dg_eventGetSuite());
	

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}