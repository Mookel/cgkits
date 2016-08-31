/*This file is generated by fileauto.sh used by test_suit.*/
/*Author: Mookel, all rights reversed.*/

#include <syslib.h>
#include "test_suit.h"

PRIVATE int test1()
{
    for(int i = 0; i <= 127; ++i) {
        char *pstr;
        char *tmp = sys_bin_to_ascii(i, 1);
        pstr = tmp;
        int  val = sys_esc(&tmp);
        printf("==>\n");
        printf("%d %s %d\n",i, pstr, val);

        tmp = sys_bin_to_ascii(i , 0);
        pstr = tmp;
        val = sys_esc(&tmp);
        printf("%d %s %d\n",i, pstr, val);

    }
    return 1;
}

PRIVATE int test2()
{
    int rval = 0;

    rval = sys_copyfile("dst.txt", "src.txt", "w");
    printf("rval = %d\n", rval);
    rval |= sys_copyfile("dst.txt", "src.txt", "a");
    printf("rval = %d\n", rval);
    //rval = sys_movefile("dst.txt", "src.txt", "a");
    printf("rval = %d\n", rval);

    return rval == FILE_ERR_NONE;
}

PRIVATE int test3()
{
    char target[128];
    int ret = sys_concat(128, target, "first", "second", "last", NULL);
    printf("size = 128, concat : %s, ret = %d\n", target, ret);
    ret = sys_concat(12, target, "first", "second", "last", NULL);
    printf("size = 12, concat : %s, ret = %d\n", target, ret);
    return 1;

}

PRIVATE int test4()
{
    char pathname[128];
    int rval = sys_searchenv("dst.txt", NULL, pathname);
    printf("%d, %s\n", rval, pathname);
    rval = sys_searchenv("clang", "PATH", pathname);
    printf("%d, %s\n", rval, pathname);

    return 1;
}

PRIVATE int test5()
{
    FILE *output = fopen("dst.txt", "w+");
    if(!output) {
        fprintf(stdout, "Open dst.txt failed.\n");
    }

    FILE *fp = sys_driver_1(output, 1, "src.txt");
    printf("addr of fp : %p\n", fp);

    return 1;

}

int __syslib_test__()
{
    int rval = 1;

    rval &= test1();
    //rval &= test2();
    rval &= test3();
    rval &= test4();
    //rval &= test5();

    return rval;
}
