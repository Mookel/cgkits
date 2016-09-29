//
// Created by Mookel on 16/9/27.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// acts.h : 
//

#ifndef CGKITS_ACTS_H
#define CGKITS_ACTS_H

void      init_acts();
int       problems();
void      first_sym();
void      add_synch(char *name);
void      new_rhs();
void      add_to_rhs(char *object, bool is_an_action, int action_lineno);
SYMBOL_S *make_term(char *name);
SYMBOL_S *new_nonterm(char *name, bool is_lhs);

#endif //CGKITS_ACTS_H
