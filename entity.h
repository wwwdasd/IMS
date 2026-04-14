#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#define Max 100

typedef struct {
	int id; //商品id仅用作查找，用户不能修改和查看
	int code; //商品编号 用户只读
	char name[Max]; //商品名称
	char price[10];  //价格
	int number; //库存量
}Entity;

typedef struct Node {
	Node* front;
	Entity val;
	Node* next;
};
typedef struct Node Node;

#endif // !ENTITY_H

