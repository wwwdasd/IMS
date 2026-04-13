#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

#include "myfiledb.h"

#define Max 100

typedef struct {
	int id; //商品id仅用作查找，用户不能修改和查看
	int code; //商品编号 用户只读
	char name[Max]; //商品名称
	char price[10];  //价格
	int number; //库存量
}Entity;

typedef struct Node{
	Node* front;
	Entity val;
	Node* next;
};
typedef struct Node Node;

/*
基础操作
*/

//初始化双链表
void initNode(Node*& node);

//得到当前商品的唯一编号，所有商品编号都唯一
int getCode(int length, int id);

//进货 id自增，价格大于等于0，若已存在则库存加1，成功返回 true 
bool addEntity(Node*& node, MYDB*& db);

//出货 复用查询  根据id删除，库存相应减少,若库存为零则直接删除
bool deleteByindex(Node*& node, int number, MYDB*& db);

//查询 用商品名查询 返回对应对象, 未查到返回0；
Node* findByName(Node* node,char name[]);

//用商品编号查询；
Node* findByCode(Node* node, int code);

//修改 复用查询 根据商品编号找到对应商品,成功返回 true 
bool modifyEntity(Node*& node, int code, MYDB*& db);

//根据id输出
void showByName(Node* node, char name[]);

//打印全部
void showAll(Node* node);


/*
文件存储操作
*/

//序列化存Entity实体
void saveGoodsSerialize(MYDB*& db, Entity* goods);

//反序列化获取Entity实体
void getGoodsSerialize(MYDB*& db, Node*& node);

//修改实体
void updateGoodsSerialize(MYDB*& db, Entity* goods);

//删除实体
void deleteGoodsSerialize(MYDB*& db, Entity* goods);


int main() {
	int switchFlag;
	bool whileFlag=true;
	Entity* entity;
	Node* node;
	Node* tmp;
	initNode(node);
	int code;
	char fname[Max];

	//初始化数据库文件
	MYDB* root;
	initRoot(root);
	addDB(root, "IMS");
	MYDB* db = openDataBase(root, "IMS");

	getGoodsSerialize(db, node);

	while (whileFlag) {
		printf("\t\t商品库存管理系统\n");
		printf("1: 进货，2：出货，3：修改商品信息，4：查看全部商品，5：查询商品，6：删除所有商品，7: 退出\n");
		scanf("%d", &switchFlag);
		switch (switchFlag) {
		case 1:
			if (!addEntity(node,db)) {
				printf("添加失败\n");
				return 1;
			} else {
				printf("添加成功\n");
				break;
			}
		case 2:
			int number;
			char name[Max];
			printf("请输入你要出货的商品：");
			scanf("%s", fname);
			// tmp为要出货的商品
			tmp = findByName(node, fname);
			if (tmp == NULL) {
				printf("未找到该商品，请核对后重试");
				break;
			}
			printf("请输入你要出货的数量：");
			scanf("%d", &number);
			
			deleteByindex(tmp, number,db);

			break;
		case 3:
			printf("请输入要修改商品的编号: ");
			scanf("%d", &code);
			modifyEntity(node,code,db);
			break;
		case 4:
			showAll(node);
			break;
		case 5:
			printf("请输入查找商品名称：");
			scanf("%s", name);
			showByName(node, name);
			break;
		case 6:
			node->next = NULL;
			claer(db);
			printf("success：删除成功");
			break;
		case 7:
			whileFlag = false;
			closeDB(db);
			printf("程序退出\n");
			return 0;
			break;
		default:
			whileFlag = false;
			printf("程序异常退出\n");
			return 1;
		}
		printf("\n\n\n");
	}
	return 0;
}


void initNode(Node*& node) {
	node = (Node*)malloc(sizeof(Node));
	node->front = NULL;
	node->val.id = -1;
	node->val.code = 0;
	node->val.name[0] = NULL;
	node->val.price[0] = NULL;
	node->val.number = 0;
	node->next = NULL;
}

bool addEntity(Node*& node, MYDB*& db) {
	//复用getLength 获取最新长度,让id自增；
	int length = count(db);
	//新的数据和空间
	Node* newNode =(Node*)malloc(sizeof(Node));
	Entity* val = new Entity;
	val->id = length+1;
	val->code = getCode(length, val->id);
	printf("请输入商品名称：");
	scanf("%s", val->name);
	getchar();
	printf("请输入商品价格：");
	scanf("%s", val->price);
	getchar();
	printf("请输入商品库存：");
	scanf("%d", &val->number);

	if (val->price[0] == '-') {
		printf("价格不能为负数");
		return false;
	}


	//如果商品名称已存在则只加库存，若价格不一致则询问保留
	Node* tmp = findByName(node,val->name);
	if (tmp!=NULL && !strcmp(val->name, tmp->val.name)) {
		printf("该商品已存在，仅增加库存值\n");
		//如果存在则库存相加
		val->number = val->number + tmp->val.number;
		//如果两次价格不同则询问保存
		if (strcmp(tmp->val.price,val->price)) {
			int flag = 0;
			printf("新价格与旧价格不同，保存哪个价格\n");
			printf("1: 新价格：%s \t2: 旧价格：%s\n", val->price, tmp->val.price);
			scanf("%d", &flag);
			int length = 0;
			switch (flag) {
			case 1:
				//保存新价格
				tmp->val.number = val->number;
				length=strlen(tmp->val.price);
				for (int t = 0; t < length; t++) {
					tmp->val.price[t] = val->price[t];
				}
				break;
			case 2:
				//保存旧价格
				tmp->val.number = val->number;
				break;
			default:
				printf("异常退出，不保存此次新数据");
				return false;
			}
			updateGoodsSerialize(db, val);
			return true;
		}

		//只修改找到对象的库存值，其余不变;
		tmp->val.number = val->number;
		updateGoodsSerialize(db, &tmp->val);
		return true;
	}

	newNode->val = *val;
	//尾插法
	Node* p=node;
	while (p->next!=NULL) {
		//p指向最后一个节点
		p = p->next;
	}

	newNode->next = p->next;
	newNode->front = p;
	p->next = newNode;

	saveGoodsSerialize(db, &newNode->val);
	return true;
}

void showByName(Node* node,char name[]) {
	Node* tmp = findByName(node, name);
	printf("该商品信息为：\n");
	printf("商品编号：%d\t 商品名称：%s\t 价格：%s\t 库存量：%d\n", tmp->val.code, tmp->val.name, tmp->val.price, tmp->val.number);
}

void showAll(Node* node) {
	Node* tmp = node->next;
	printf("所有商品信息为：\n");
	printf("%-15s %-25s %-10s %-10s\n", "商品编号", "商品名称", "价格", "库存量");
	while (tmp!=NULL) {
		printf("%-16d%-26s%-11s%-10d\n", tmp->val.code, tmp->val.name, tmp->val.price, tmp->val.number);
		tmp = tmp->next;
	}
}

Node* findByName(Node* node,char name[]) {
	Node* tmp = node;
	while (tmp!=NULL) {
		if (!strcmp(name,tmp->val.name)) {
			break;
		}
		tmp = tmp->next;
	}
	return tmp;
}

Node* findByCode(Node* node, int code) {
	Node* tmp = node;
	while (tmp != NULL) {
		if (tmp->val.code==code) {
			break;
		}
		tmp = tmp->next;
	}
	return tmp;
}

int getCode(int length, int id) {
	int result = 2026+(id + 5*length) * 3;
	return result;
}

bool modifyEntity(Node*& node, int code, MYDB*& db) {
	Entity* val = new Entity;
	Node* tmp = findByCode(node,code);
	if (tmp==NULL) {
		printf("为找到该商品，请核对编号后重试\n");
		return false;
	}
	int flag,length;
	printf("请选择修改项: 1.商品名称 2.商品价格 3.商品库存 4.退出\n");
	scanf("%d", &flag);
	switch (flag) {
	case 1:
		printf("请输入新名称：  ");
		scanf("%s", val->name);
		length = strlen(tmp->val.name);
		for (int i = 0; i < length; i++) {
			tmp->val.name[i] = val->name[i];
		}
		showByName(tmp, tmp->val.name);
		break;
	case 2:
		printf("请输入新价格：  ");
		scanf("%s", val->price);
		length = strlen(tmp->val.price);
		for (int i = 0; i < length; i++) {
			tmp->val.price[i] = val->price[i];
		}
		showByName(tmp, tmp->val.name);
		break;
	case 3:
		printf("请输入新库存量：  ");
		scanf("%d", &val->number);
		tmp->val.number = val->number;
		showByName(tmp, tmp->val.name);
		break;
	default:
		break;
	}
	updateGoodsSerialize(db, &tmp->val);
	return true;
}

bool deleteByindex(Node*& node, int number, MYDB*& db) {
	//优先判断库存是否充足
	if (node->val.number<number) {
		printf("当前库存不足，当前库存为 %d ", node->val.number);
		return false;
	}
	//库存足够在进行下一步
	Entity* val = new Entity;
	val->number = node->val.number - number;
	char flag;

	if (val->number==0) {
		//当前库存为零，询问是否保存
		printf("出库后库存为零，是否保存当前商品：Y/N\n");
		getchar();
		scanf("%c", &flag);
		switch (flag) {
		case 'Y':
		case 'y':
			node->val.number = val->number;
			updateGoodsSerialize(db, &node->val);
			return true;
			break;
		case 'N':
		case 'n':
			node->front->next = node->next;
			node->next = NULL;
			deleteGoodsSerialize(db, &node->val);
			free(node);
			return true;
			break;
		default:
			break;
		}
		
	} else {
		node->val.number = val->number;
		printf("当前剩余库存：%d", node->val.number);
		updateGoodsSerialize(db, &node->val);
		return true;
	}
}



void saveGoodsSerialize(MYDB*& db, Entity* goods) {

	char key[Max];
	sprintf(key, "goods_%d", goods->id);

	char value[sizeof(Entity)];
	sprintf(value, "id:%d,code:%d,name:%s,price:%s,number:%d", goods->id, goods->code, goods->name, goods->price, goods->number);

	set(db, key, value);

}

void getGoodsSerialize(MYDB*& db, Node*& node) {
	Node* nodeTmp = node->next;
	KV_Node* KVtmp = db->DataBase.KV_list.next;

	while (KVtmp != NULL) {
		Node* newNode = new Node;
		Entity* goods = new Entity;
		char* value = get(db, KVtmp->key);
		sscanf(value, "id:%d,code:%d,name:%[^,],price:%[^,],number:%d", &goods->id, &goods->code, goods->name, goods->price, &goods->number);
		KVtmp = KVtmp->next;
		//写入内存

		newNode->val = *goods;
		Node* p = node;
		while (p->next != NULL) {
			//p指向最后一个节点
			p = p->next;
		}

		newNode->next = p->next;
		newNode->front = p;
		p->next = newNode;

	}
}

void updateGoodsSerialize(MYDB*& db, Entity* goods) {

	char key[Max];
	sprintf(key, "goods_%d", goods->id);

	char value[sizeof(Entity)];
	sprintf(value, "id:%d,code:%d,name:%s,price:%s,number:%d", goods->id, goods->code, goods->name, goods->price, goods->number);

	update(db, key, value);

}

void deleteGoodsSerialize(MYDB*& db, Entity* goods) {
	char* goodsName = new char;
	sprintf(goodsName, "goods_%d", goods->id);
	del(db, goodsName);
}