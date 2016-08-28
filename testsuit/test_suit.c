//
// Created by Mookel on 16/8/28.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// test_suit.c : This file is used by test suit.It
// will run all test, if failed, it will exit.
//

#include <stdio.h>

#define MAX_TEST_SUIT 256

typedef int (*TestFuncType)();
static int g_TestCount = 0;

struct {
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

int main()
{
    int result = 1;
    struct {
        char *fail_name;
    }FailedTest[MAX_TEST_SUIT];
    int failTestNum = 0;
    
    ADD_TEST("stack", stack);
    TEST_ADD_FLAG(TRUE);
    for(int i = 0;i < g_TestCount; ++i) {
        printf("----------%s testing begin-------\n", TestSuit[i].name);
        result = TestSuit[i]._func_();
        if(result == 0) FailedTest[failTestNum++].fail_name = TestSuit[i].name;
        printf("----------%s testing end  -------\n", TestSuit[i].name);
    }
    
    printf("\n-----------Testing result---------\n");
    printf("Total Test count       : %d\n", g_TestCount);
    printf("Total failed Test Count: %d\n", failTestNum);
    printf("Failed test name       : \n");
    for(int i = 0;i < failTestNum; ++i){
        printf("\"%s\"  ", FailedTest[i].fail_name);
        if(i != 0 && i % 6 == 0) printf("\n");
    }
    printf("\n----------Testing result end------\n");

    return 0;
}