//
// Created by Mookel on 16/8/28.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// test_suit.h : 
//

#ifndef CGKITS_TEST_SUIT_H
#define CGKITS_TEST_SUIT_H

#include <string.h>

#define TEST_SUCCESS  1
#define TEST_FAILED   0

#define TEST_ASSERT_INT(a, b) {if((a) != (b)) return 0;}
#define TEST_ASSERT_STR(a, b) {if(strcmp((a), (b)) != 0) return 0;}

#endif //CGKITS_TEST_SUIT_H
