
#include "TestResult.h"
#include "Failure.h"

#include <stdio.h>

TestResult::TestResult()
    : failureCount(0)
{
}


void TestResult::testsStarted()
{
}


void TestResult::addFailure(const Failure& failure)
{
    fprintf(stdout, "FAILED\n");
    fprintf(stdout, "%s%s%s%s%ld%s%s\n",
            "Failure: \"",
            failure.message.asCharString(),
            "\" " ,
            "line ",
            failure.lineNumber,
            " in ",
            failure.fileName.asCharString());

    failureCount++;
}


void TestResult::testsEnded()
{
    if (failureCount > 0)
        fprintf(stdout, "%d Tests failed\n", failureCount);
    else
        fprintf(stdout, "Everything OK!\n");
}
