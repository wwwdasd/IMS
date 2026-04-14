#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>

#include "myfiledb.h"
#include "entity.h"

#define MAX 100

//操作
typedef enum {
	SAVE,
	UPDATA,
	DELETE
}OpType;

//操作结构体
typedef struct {
	Entity oldEntity;
	OpType type;//操作类型
	int id;//被操作id
}Operation;


//栈定义
typedef struct stack {
	Operation operation[MAX];
	int top;
}Stack;


void initStack(Stack*& stack) {
	stack = new Stack;
	stack->top = -1;
}

//入栈
bool push(Stack*& stack, Operation op) {
	if (stack->top == MAX - 1) {
		return  false;
	}
	stack->top++;
	stack->operation[stack->top] = op;
	return true;
}

//出栈
Operation* pop(Stack*& stack) {
	Operation* op;
	if (stack->top < 0) {
		return NULL;
	}
	op = &stack->operation[stack->top];
	stack->top--;
	return op;
}

/*
撤销
*/
bool undo(Stack*& stack, Node*& node, MYDB*& db) {
	//stack中有旧数据
	Node* nodeTmp = node;
	Node* newNode = new Node;
	char* goodsName = new char;
	char key[Max];
	char value[sizeof(Entity)];

	Operation* op = new Operation;
	op = pop(stack);

	if (op==NULL) {
		return false;
	}


	switch (op->type) {
	case SAVE:
		//保存逆操作是删除
		//先在当前node里找到要删除的项
		//id唯一，利用id找
		//先删文件里的

		sprintf(goodsName, "goods_%d", op->id);
		del(db, goodsName);

		while (nodeTmp != NULL) {
			if (nodeTmp->val.id == op->oldEntity.id) {
				break;
			}
			nodeTmp = nodeTmp->next;
		}
		//跳出则tmp为当前要删除的项
		if (nodeTmp == NULL) {
			return false;
		}
		nodeTmp->front->next = nodeTmp->next;
		if (nodeTmp->next!=NULL) {
			nodeTmp->next->front = nodeTmp->front;
		}
		free(nodeTmp);
		return true;
		break;
	case UPDATA:
		//更改逆操作是改回旧数据
		//用旧数据覆盖新数据

		while (nodeTmp != NULL) {
			if (nodeTmp->val.id == op->oldEntity.id) {
				break;
			}
			nodeTmp = nodeTmp->next;
		}

		if (nodeTmp == NULL) {
			return false;
		}

		nodeTmp->val = op->oldEntity;

		//写入文件
		sprintf(key, "goods_%d", op->id);
		sprintf(value, "id:%d,code:%d,name:%s,price:%s,number:%d", op->id, op->oldEntity.code, op->oldEntity.name, op->oldEntity.price, op->oldEntity.number);

		update(db, key, value);

		break;
	case DELETE:
		//删除的逆操作是添加

		newNode->val = op->oldEntity;

		//找到原位置的前一个节点，id = oldEntity.id-1
		sprintf(key, "goods_%d", op->id);
		sprintf(value, "id:%d,code:%d,name:%s,price:%s,number:%d", op->id, op->oldEntity.code, op->oldEntity.name, op->oldEntity.price, op->oldEntity.number);
		set(db, key, value);

		//while (nodeTmp != NULL) {
		//	if (nodeTmp->val.id == op->oldEntity.id - 1) {
		//		break;
		//	}
		//	nodeTmp = nodeTmp->next;
		//}

		//if (nodeTmp == NULL) {
		//	return false;
		//}

		newNode->front = nodeTmp;
		newNode->next = nodeTmp->next;
		nodeTmp->next = newNode;

		//写入文件


		break;
	default:
		return false;
		break;
	}

}