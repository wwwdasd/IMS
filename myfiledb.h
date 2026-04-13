#pragma once
#ifndef MYFILEDB_H
#define MYFILEDB_H


#define MYDB Root
#define VAL_LENGTH 200
#define NAME_LENGTH 20


#include<stdio.h>
#include<string.h>
#include<stdlib.h>



/*
文件数据库，实现文件读写，可再其他程序复用
简化其他程序的读写

用链表链接每个文件
*/

typedef struct KV_Node {
	char key[VAL_LENGTH];
	char value[VAL_LENGTH];
	KV_Node* next;
};
typedef struct KV_Node KVnode;


//一个文件是一个库
typedef struct {
	//文件结构体
	char name[NAME_LENGTH];
	FILE* file;//文件指针
	int status;
	KVnode KV_list;//文件状态 1为打开 0为关闭
}DataBase;

typedef struct Root {
	DataBase DataBase;
	Root* next;
};
typedef struct Root Root;



//函数声明，方便复用

//初始化数据库根目录
static inline void initRoot(Root*& DB);
//新建和初始化库文件
static inline bool addDB(Root*& DB, const char* name);
//打开库
static inline Root* openDataBase(Root*& DB, const char* dbName);
//一行一行读取到内存中，读取速度快
static inline bool readToMemory(FILE*& dbFile, Root*& root);
//关闭所有库
static inline bool closeDB(Root*& DB);
//删除库
static inline bool delDB(Root*& rootDB, const char* DBname);
//判断库是否存在
static inline bool exisitsDB(Root* DB, const char* DBname);
//新增键值对
static inline bool set(Root*& DB, const char* key, const char* value);
//获取值
static inline char* get(Root*& DB, const char* key);
//删除键值对
static inline bool del(Root*& DB, const char* key);
//输出当前库内所有键
static inline void scan(Root*& DB);
//修改键值对
static inline bool update(Root*& DB, const char* key, const char* newValue);
//清空库内所有键值对
static inline bool claer(Root*& DB);
//判断键是否存在
static inline bool exists(Root*& DB, const char* key);
// 查键值对数量
static inline int count(Root*& DB);
// 列出所有键
static inline void list(Root*& DB);




static inline void initRoot(Root*& root) {
	root = new Root;
	root->DataBase.name[0] = NULL;
	root->DataBase.file = NULL;
	root->DataBase.status = -1;
	root->DataBase.KV_list.key[0] = NULL;
	root->DataBase.KV_list.value[0] = NULL;
	root->DataBase.KV_list.next = NULL;
	root->next = NULL;
}

/*
库操作
*/

//新建和初始化库文件
static inline bool addDB(Root*& root, const char* name) {

	if (exisitsDB(root, name)) {
		return false;
	}

	Root* newNode = new Root;
	char tmpName[NAME_LENGTH];
	strcpy(tmpName, name);
	//sprintf(tmpName, "%sDB.mydb", name);
	sprintf(tmpName, "%sDB.txt", name);
	FILE* file = fopen(tmpName, "a+");
	strcpy(newNode->DataBase.name, name);
	newNode->DataBase.file = file;
	newNode->DataBase.KV_list.key[0] = NULL;
	newNode->DataBase.KV_list.value[0] = NULL;
	newNode->DataBase.KV_list.next = NULL;
	newNode->DataBase.status = 0;
	newNode->next = root->next;
	root->next = newNode;
	fclose(file);
	return true;
}

//打开库
static inline Root* openDataBase(Root*& root, const char* DBName) {
	if (!exisitsDB(root, DBName)) {
		return NULL;
	}
	FILE* dbFile = NULL;
	Root* tmp = root->next;
	char dbname[NAME_LENGTH];
	strcpy(dbname, (char*)DBName);
	while (tmp != NULL) {
		if (!strcmp(tmp->DataBase.name, dbname)) {
			if (tmp->DataBase.status == 0) {
				sprintf(dbname, "%sDB.mydb", tmp->DataBase.name);
				//sprintf(dbname, "%sDB.txt", tmp->DataBase.name);
				dbFile = fopen(dbname, "a+");
				tmp->DataBase.status = 1;
				break;
			} else if (tmp->DataBase.status == 1) {
				dbFile = tmp->DataBase.file;
				break;
			}
		}
		tmp = tmp->next;
	}

	readToMemory(dbFile, tmp);
	return tmp;
}

//一行一行读取到内存中，读取速度快
static inline bool readToMemory(FILE*& dbFile, Root*& root) {
	char KV[2 * VAL_LENGTH];
	while (fgets(KV, sizeof(KV), dbFile)) {
		//去掉换行
		KV[strcspn(KV, "\n")] = 0;

		//找到“=”
		char* equal = strchr(KV, '=');
		if (!equal) {
			return false;
		}

		*equal = '\0';
		char* key = KV;
		char* value = equal + 1;

		//新的KV
		KVnode* newKV_list = new KVnode;
		strcpy(newKV_list->key, key);
		strcpy(newKV_list->value, value);

		newKV_list->next = root->DataBase.KV_list.next;
		root->DataBase.KV_list.next = newKV_list;
	}
	return true;
}

//关闭所有库
static inline bool closeDB(Root*& root) {
	Root* tmp = root->next;
	while (tmp != NULL) {
		if (tmp->DataBase.status == 1) {
			tmp->DataBase.status = 0;
			fclose(tmp->DataBase.file);
		}
		tmp = tmp->next;
	}
	return true;
}

//删除库
static inline bool delDB(Root*& rootDB, const char* DBname) {
	closeDB(rootDB);
	Root* tmp = rootDB;
	while (tmp != NULL && tmp->next != NULL) {
		if (!strcmp(DBname, tmp->next->DataBase.name)) {
			tmp->next = tmp->next->next;
			break;
		}
		tmp = tmp->next;
	}

	char dbname[NAME_LENGTH];
	sprintf(dbname, "%sDB.mydb", DBname);
	return !remove(dbname);
}

//判断库是否存在
static inline bool exisitsDB(Root* DB, const char* DBname) {
	bool flag = false;
	Root* tmp = DB;
	while (tmp != NULL && tmp->next != NULL) {
		if (!strcmp(DBname, tmp->next->DataBase.name)) {
			flag = true;
			break;
		}
		tmp = tmp->next;
	}
	return flag;
}

/*
KV操作,Node为当前数据库
*/


//新增键值对
static inline bool set(Root*& DB, const char* key, const char* value) {

	//键唯一
	KV_Node* tmp = DB->DataBase.KV_list.next;

	if (&DB->DataBase.KV_list == NULL) {
		return false;
	}

	while (tmp != NULL) {
		if (!strcmp(tmp->key, key)) {
			return false;
		}
		tmp = tmp->next;
	}


	KVnode* newKV_list = new KVnode;
	strcpy(newKV_list->key, key);
	strcpy(newKV_list->value, value);

	newKV_list->next = DB->DataBase.KV_list.next;
	DB->DataBase.KV_list.next = newKV_list;

	fprintf(DB->DataBase.file, "%s=%s\n", key, value);
	fflush(DB->DataBase.file);
	return true;
}

//获取值
static inline char* get(Root*& DB, const char* key) {
	KV_Node* tmp = DB->DataBase.KV_list.next;
	while (tmp != NULL) {
		if (!strcmp(tmp->key, key)) {
			return tmp->value;
		}
		tmp = tmp->next;
	}
	return NULL;
}

//删除键值对
static inline bool del(Root*& DB, const char* key) {
	bool flag = false;
	//在内存中删除
	KV_Node* KVtmp = &DB->DataBase.KV_list;
	while (KVtmp != NULL && KVtmp->next != NULL) {
		if (!strcmp(KVtmp->next->key, key)) {
			KVtmp->next = KVtmp->next->next;
			flag = true;
			break;
		}
		KVtmp = KVtmp->next;
	}
	if (flag) {
		//写入文件
		fclose(DB->DataBase.file);
		char dbName[NAME_LENGTH];
		//sprintf(dbName, "%sDB.txt", DB->DataBase.name);
		sprintf(dbName, "%sDB.mydb", DB->DataBase.name);
		FILE* newDBFile = fopen(dbName, "w");
		DB->DataBase.file = newDBFile;
		KVtmp = DB->DataBase.KV_list.next;
		while (KVtmp != NULL) {
			fprintf(DB->DataBase.file, "%s=%s\n", KVtmp->key, KVtmp->value);
			KVtmp = KVtmp->next;
		}
		rewind(newDBFile);
		fclose(newDBFile);
		FILE* DBFile = fopen(dbName, "a+");
		DB->DataBase.file = DBFile;
		return true;
	}
}

//输出当前库内所有键
static inline void scan(Root*& DB) {
	KV_Node* KVtmp = DB->DataBase.KV_list.next;
	while (KVtmp != NULL) {
		printf("%s = %s \n", KVtmp->key, KVtmp->value);
		KVtmp = KVtmp->next;
	}
}

//修改键值对
static inline bool update(Root*& DB, const char* key, const char* newValue) {
	KV_Node* tmp = DB->DataBase.KV_list.next;
	bool flag = false;
	while (tmp != NULL) {
		if (!strcmp(tmp->key, key)) {
			flag = true;
			break;
		}
		tmp = tmp->next;
	}
	if (flag) {
		del(DB, key);
		set(DB, key, newValue);
	}
	return flag;
}

//清空库内所有键值对
static inline bool claer(Root*& DB) {
	DB->DataBase.KV_list.next = NULL;
	fclose(DB->DataBase.file);
	char dbName[NAME_LENGTH];
	//sprintf(dbName, "%sDB.txt", DB->DataBase.name);
	sprintf(dbName, "%sDB.mydb", DB->DataBase.name);
	FILE* fp = fopen(dbName, "w");
	fclose(fp);
	DB->DataBase.file = fopen(dbName, "r");
	return true;
}

//判断键是否存在
static inline bool exists(Root*& DB, const char* key) {
	KV_Node* tmp = DB->DataBase.KV_list.next;
	while (tmp != NULL) {
		if (!strcmp(tmp->key, key)) {
			return true;
		}
		tmp = tmp->next;
	}
	return false;
}

// 查键值对数量
static inline int count(Root*& DB) {
	int cnt = 0;
	KV_Node* tmp = DB->DataBase.KV_list.next;

	while (tmp != NULL) {
		cnt++;
		tmp = tmp->next;
	}

	return cnt;
}

// 列出所有键
static inline void list(Root*& DB) {
	KV_Node* tmp = DB->DataBase.KV_list.next;
	while (tmp != NULL) {
		printf("%s\n", tmp->key);
		tmp = tmp->next;
	}
}


#endif // !MYFILEDB_H
