#pragma once
#ifndef STACK_H
#define STACK_H

#define Stack Stack
#define MAX 100

#include "entity.h"

//操作
typedef enum {
	SAVE,
	UPDATA,
	DELETE
}OpType;

//操作结构体
typedef struct {
	Entity oldEntity; //操作数据
	OpType type;//操作类型
	int id;//被操作id
}Operation;


//栈定义
typedef struct stack {
	Operation operation[MAX];
	int top;
}Stack;


inline void initStack(Stack*& stack);
inline bool push(Stack*& stack, Operation op);
Operation* pop(Stack*& stack);

bool undo(Stack*& stack, Node*& node, MYDB*& db);

#endif // STARCK_H

