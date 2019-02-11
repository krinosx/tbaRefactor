
#include "CuTest.h"
#include "../src/structs.h"

/*-------------------------------------------------------------------------*
 * CuString Test
 *-------------------------------------------------------------------------*/
/** Function prototypes for code being tested */
struct boot_cmd_args_data* parseCommandLine(int argc, char **argv);


/**
* Testing code
*/

/**
* Check if we can get the default file name if noone is provided with -f

 printf("Usage: %s [-c] [-m] [-q] [-r] [-s] [-d pathname] [port #]\n"
		  "  -c             Enable syntax check mode.\n"
		  "  -d <directory> Specify library directory (defaults to 'lib').\n"
		  "  -h             Print this command line argument help.\n"
		  "  -m             Start in mini-MUD mode.\n"
		  "  -f<file>       Use <file> for configuration. Note: There is no space after -f flag\n"
		  "  -o<file>       Write log to <file> instead of stderr. There is no space after -o flag \n"
		  "  -q             Quick boot (doesn't scan rent for object limits)\n"
		  "  -r             Restrict MUD -- no new players allowed.\n"
		  "  -s             Suppress special procedure assignments.\n"
		  "  -t				Run unity test before loading the world. Quit if any tests fail.\n"
		  "  -T				Run unity test before loading the world. Quit after test execution (even if all pass).\n"
		  " Note:		These arguments are 'CaSe SeNsItIvE!!!'\n",
		  argv[0]
	  );
	  exit(0);
*/
//struct boot_cmd_args_data;


void test_parseCommandLine_fullargs(CuTest* tc)
{
	int argc = 14;
	char **argv = (char**)malloc(sizeof(char*) * argc);
	
	/* Executing with a mini mode enabled */
	argv[0] = "executable_filename";
	argv[1] = "-c";
	argv[2] = "-d";
	argv[3] = "c:\\lib\\world";
	argv[4] = "-h";
	argv[5] = "-m";
	argv[6] = "-f";
	argv[7] = "c:\\configfile.cfg";
	argv[8] = "-o";
	argv[9] = "c:\\logfile.txt";
	argv[10] = "-q";
	argv[11] = "-r";
	argv[12] = "-s";
	argv[13] = "-t";

	struct boot_cmd_args_data* cmdData = parseCommandLine(argc, argv);
 
	CuAssertIntEquals_Msg(tc, "Fail to parse -c argument", 1, cmdData->syntaxCheckMode);
	
	CuAssertPtrNotNullMsg(tc, "Fail to parse -d command (library/data folder)", cmdData->dataFolder);
	CuAssertStrEquals_Msg(tc, "Unexpected value returned fom -d <folder> parameter! ", "c:\\lib\\world", cmdData->dataFolder);

	CuAssertIntEquals_Msg(tc, "Fail to parse -h argument", 1, cmdData->displayHelp);
	CuAssertIntEquals_Msg(tc, "Fail to parse -m argument", 1, cmdData->miniMudMode);

	CuAssertPtrNotNullMsg(tc, "Fail to parse -f command (configuration file)", cmdData->configFileName);
	CuAssertStrEquals_Msg(tc, "Unexpected value returned fom -f <config_file_name> parameter! ", "c:\\configfile.cfg", cmdData->configFileName);

	CuAssertPtrNotNullMsg(tc, "Fail to parse -o command (log file)", cmdData->logFileName);
	CuAssertStrEquals_Msg(tc, "Unexpected value returned fom -o <log_file_Name> parameter! ", "c:\\logfile.txt", cmdData->logFileName);

	CuAssertIntEquals_Msg(tc, "Fail to parse -q argument", 1, cmdData->noRentCheckMode);
	CuAssertIntEquals_Msg(tc, "Fail to parse -r argument", 1, cmdData->restrictMode);
	CuAssertIntEquals_Msg(tc, "Fail to parse -s argument", 1, cmdData->noSpecialsMode);
	
	CuAssertIntEquals_Msg(tc, "Fail to parse -t argument", 1, cmdData->runUnityTest);
	CuAssertIntEquals_Msg(tc, "Fail to parse -t argument", 0, cmdData->exitAfterTests);

}

/**
 * Export function to aggregate all tests. This function must be 
 * included in testrunner.c
*/
CuSuite* CommGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_parseCommandLine_fullargs);
	
	return suite;
}