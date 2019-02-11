#include "CuTest.h"


/*-------------------------------------------------------------------------*
 * CuString Test
 *-------------------------------------------------------------------------*/

char* resolveConfigFileName(int argc, char **argv, char* defaultConfigFile);

void TestResolvConfigFileName(CuTest* tc)
{


	//char* resolveConfigFileName(int argc, char **argv, char* defaultConfigFile) {

	char **buf1 = (char**) malloc(sizeof(char*)*3);

	char * opt1 = "-m";
	char * opt2 = "-o";
	char * opt3 = "C:\\log.txt";
	
	buf1[1] = opt1;
	buf1[2] = opt2;
	buf1[3] = opt3;



	/* Teste passando parametros, porem sem o arquivo de log*/
	char * result = resolveConfigFileName(4, buf1, "c:\\teste.txt");
	
	CuAssertPtrNotNullMsg(tc, "Arquivo nao encontrado", result);
	CuAssertStrEquals_Msg(tc, "Unexpected file name returned! ", "c:\\teste.txt", result);


	buf1[2] = "-f";
	buf1[3] = "C:\\config\\config.txt";
	char * result1 = resolveConfigFileName(4, buf1, "c:\\teste.txt");

	CuAssertPtrNotNullMsg(tc, "Arquivo nao encontrado", result1);
	CuAssertStrEquals_Msg(tc, "Unexpected file name returned! ", "C:\\config\\config.txt", result1);

	//CuString* str = CuStringNew();
	//CuAssertTrue(tc, 0 == str->length);
	//CuAssertTrue(tc, 0 != str->size);
	//CuAssertStrEquals(tc, "", str->buffer);
}









CuSuite* CuStringGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, TestResolvConfigFileName);
	
	return suite;
}


void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	//CuSuiteAddSuite(suite, CuGetSuite());
	CuSuiteAddSuite(suite, CuStringGetSuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}