/*This file is generated by fileauto.sh used by test_suit.*/
/*Author: Mookel, all rights reversed.*/

#include <hash.h>
#include "test_suit.h"

typedef struct STAB_{
    char name[32];
    char str[16];
    unsigned  hash_key;
    int count;
}STAB;

void tab_print(char *name, ...)
{
    printf(name);
    printf("\n");
}

#define TEST_SYM_SIZE 20

int get_word(char *buf)
{
    static int word_nc = TEST_SYM_SIZE;
    int num_letters, let;

    if(--word_nc < 0) return 0;

    while((num_letters = rand() % 16) < 3);
    while(--num_letters >= 0)
    {
        let = (rand() % 26) + 'a';
        *buf++ = (rand() % 10) ? let : toupper(let);
    }

    *buf = '\0';
    return 1;
}

int __hash_test__()
{
    char word[80];
    STAB *allsym[TEST_SYM_SIZE];
    int  sym_count = 0;
    STAB *psym = NULL;

    HASH_TAB_S *p_tab;

    p_tab = hash_make_tab(1024 , hash_pjw, strcmp);

    while(get_word(word)){
        if(psym = (STAB *)hash_find_sym(p_tab, word)){
            if(strcmp(psym->str, "123456789abcdef") ||
                    psym->hash_key != hash_add(word)){
                fprintf(stdout, "NODE HAS BEEN ADULTERATED\n");
                return 0;
            }

        } else {
            psym = hash_new_sym(sizeof(STAB));
            strncpy(psym->name, word, 32);
            strcpy(psym->str, "123456789abcdef");
            psym->hash_key = hash_add(word);
            hash_add_sym(p_tab, psym);
            allsym[sym_count++] = psym;
        }
    }

    hash_dump(p_tab);
    fprintf(stdout, "==> without sorting: \n");
    hash_print_tab(p_tab, tab_print, NULL, 0);
    fprintf(stdout, "==> with sorting: \n");
    hash_print_tab(p_tab, tab_print, NULL, 1);

    for(int i = 0;i < sym_count; ++i){
        hash_free_sym(allsym[i]);
    }

    hash_free_tab(p_tab);
    return 1;
}
