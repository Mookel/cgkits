/*This file is generated by fileauto.sh used by test_suit.*/
/*Author: Mookel, all rights reversed.*/

#include <yystack.h>
#include "test_suit.h"

#undef yystk_err
#define yystk_err(o) 0

int __yystack_test__()
{
    yystk_dcl(stk, int , 128);

    for (int i = 0; i < 128; i++) {
        yypush(stk, i);
        printf("size : %d\n", yystk_ele(stk));
    }
    TEST_ASSERT_INT(yystk_ele(stk), 128);
    TEST_ASSERT_INT(yystk_full(stk), 1);

    yypopn(stk, 128);
    TEST_ASSERT_INT(yystk_ele(stk), 0);
    TEST_ASSERT_INT(yystk_empty(stk), 1);

    for(int i = 0;i < 128; i++) {
        yypush(stk, i);
    }
    for(int i = 0;i < 128; ++i){
        printf("pop value: %d\n", yypop(stk));
    }
    TEST_ASSERT_INT(yystk_empty(stk), 1);
    yypush(stk, 1);
    yypush(stk, 2);
    TEST_ASSERT_INT(yystk_ele(stk), 2);
    yystk_clear(stk);
    TEST_ASSERT_INT(yystk_ele(stk), 0);

    return TEST_SUCCESS;
}
