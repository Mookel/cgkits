//
// Created by Mookel on 16/8/28.
// Email : ltp0709@sina.com
// Copyright (c) 2016 jlu.edu. All rights reserved.
// test_suit.c : This file is used by test suit.It
// will run all test, if failed, it will exit.
//

#include <stdio.h>
#include "test_suit.h"

#define MAX_TEST_SUIT 256

typedef int (*TestFuncType)();
static int g_TestCount = 0;

struct UnitTest{
    char *name;
    TestFuncType  _func_;
}TestSuit[MAX_TEST_SUIT];

#define ADD_TEST(s, f) extern int __##f##_test__(); \
                       {\
                            int idx = g_TestCount++;\
                            TestSuit[idx].name = s; \
                            TestSuit[idx]._func_ = __##f##_test__; \
                       }

#define TEST_ADD_FLAG(o)

int run_test(struct UnitTest *ptest)
{
    int result = 0;
    printf("\n----------%s testing begin-------\n", ptest->name);
    result = ptest->_func_();
    printf("\n----------%s testing end  -------\n", ptest->name);
    return result;
}

int main(int argc, char **argv)
{
    int result = 1;
    struct {
        char *fail_name;
    }FailedTest[MAX_TEST_SUIT];
    int failTestNum = 0;
    int testRunCnt = 0;

    ADD_TEST("stack", stack);
    ADD_TEST("yystack", yystack);
    ADD_TEST("hash", hash);
    ADD_TEST("set", set);
    ADD_TEST("syslib", syslib);
    TEST_ADD_FLAG(TRUE);

    if(argc != 1){ //run one test.
        for(int i = 1;i < argc; ++i){
            for(int j = 0;j < g_TestCount; ++j){
                if(strcmp(TestSuit[j].name, argv[i]) == 0){
                    result = run_test(&TestSuit[j]);
                    ++testRunCnt;
                    if(result == 0) FailedTest[failTestNum++].fail_name = TestSuit[j].name;
                    break;
                }
            }
        }
    }
    else{ //or we run all test.
        for(int i = 0;i < g_TestCount; ++i) {
            result = run_test(&TestSuit[i]);
            ++testRunCnt;
            if(result == 0) FailedTest[failTestNum++].fail_name = TestSuit[i].name;
        }
    }

    printf("\n-----------Testing result---------\n");
    printf("Total Test count       : %d\n", testRunCnt);
    printf("Total failed Test Count: %d\n", failTestNum);
    printf("Failed test name       : \n");
    for(int i = 0;i < failTestNum; ++i){
        printf("\"%s\"  ", FailedTest[i].fail_name);
        if(i != 0 && i % 6 == 0) printf("\n");
    }
    printf("\n----------Testing result end------\n");

    return 0;
}
