#include "hash.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static hash_node_t ** hash_get_bucket(hash_t *hash, void *key);
static hash_node_t *hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size);
static void hash_clear_bucket(hash_t *hash, unsigned int index);


//建立一个hash表，然后将指针返回
hash_t* hash_alloc(unsigned int buckets, hashfunc_t hash_func)
{
	hash_t *hash = (hash_t *)malloc(sizeof(hash_t));
	hash->buckets = buckets;	//桶的个数
	hash->hash_func = hash_func; //hash函数
	int len = sizeof(hash_node_t *) * buckets; //数组占用的字节数
	hash->nodes = (hash_node_t **)malloc(len);
	memset(hash->nodes, 0, len);

	return hash;
}

//根据key查找value
void* hash_lookup_value_by_key(hash_t *hash, void* key, unsigned int key_size)
{
	//先查找结点
	hash_node_t *node = hash_get_node_by_key(hash, key, key_size);
	if(node == NULL)
		return NULL;
	else
		return node->value;
}

//向hash中添加结点
void hash_add_entry(hash_t *hash, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	//1. 查找bucket
	hash_node_t **buck = hash_get_bucket(hash, key);
	assert(buck != NULL);

	//2. 生成新的结点
	hash_node_t *node = (hash_node_t *)malloc(sizeof(hash_node_t));
	memset(node, 0, sizeof(hash_node_t));
	node->key = malloc(key_size);
	node->value = malloc(value_size);
	memcpy(node->key, key, key_size);
	memcpy(node->value, value, value_size);

	//3. 插入相应的链表  头插法
	if(*buck != NULL)
	{
		hash_node_t *head = *buck; //head是链表的第一个结点

		node->next = head;
		head->prev = node;
		*buck = node; //这里一定要用指针操作
	}
	else
	{
		*buck = node;
	}
}


void hash_free_entry(hash_t *hash, void *key, unsigned int key_size)
{
	//查找结点
	hash_node_t *node = hash_get_node_by_key(hash, key, key_size);
	assert(node != NULL);

	free(node->key);
	free(node->value);

	//删除结点
	if(node->prev != NULL) //不是第一个结点
	{
		node->prev->next = node->next;
	}
	else
	{
		hash_node_t **buck = hash_get_bucket(hash, key);
		*buck = node->next;
	}
	if(node->next != NULL) //还有下一个结点
		node->next->prev = node->prev;	

	free(node);
}

//清空hash表
void hash_clear_entry(hash_t *hash)
{
	unsigned int i;
	for(i = 0; i < hash->buckets; ++i)
	{
		hash_clear_bucket(hash, i);
		hash->nodes[i] = NULL;
	}
}

//销毁hash表
void hash_destroy(hash_t *hash)
{
	hash_clear_entry(hash);
	free(hash->nodes);
	free(hash);
}

//根据key获取bucket
static hash_node_t **hash_get_bucket(hash_t *hash, void *key)
{
	unsigned int pos = hash->hash_func(hash->buckets, key);
	assert(pos < hash->buckets);

	return &(hash->nodes[pos]);
}

//根据key获取node结点
static hash_node_t *hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size)
{
	//1. 获取bucket
	hash_node_t **buck = hash_get_bucket(hash, key);
	assert(buck != NULL);

	//2. 查找元素
	hash_node_t *node = *buck; //指向第一个元素
	while(node != NULL && memcmp(node->key, key, key_size) != 0)
		node = node->next;

	return node;	//包含了三种情况
}

//清空某个bucket
static void hash_clear_bucket(hash_t *hash, unsigned int index)
{
	assert(index < hash->buckets); //防止越界
	hash_node_t *node = hash->nodes[index];
	hash_node_t *tmp = node;
	while(node)
	{
		tmp = node->next;
		free(node->key);
		free(node->value);
		free(node);
		node = tmp;
	}
	hash->nodes[index] = NULL;
}

