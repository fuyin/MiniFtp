#include "ftp_assist.h"
#include "common.h"
#include "sysutil.h"
#include "session.h"
#include "configure.h"
#include "parse_conf.h"
#include "ftp_code.h"
#include "command_map.h"
#include "hash.h"

unsigned int num_of_clients = 0; //表示连接数目

hash_t *ip_to_clients;
hash_t *pid_to_ip;

static void handle_sigchld(int sig);
static unsigned int hash_func(unsigned int buckets, void *key);
//获取当前ip的数目，并且+1
static unsigned int add_ip_to_hash(uint32_t ip);


void check_permission()
{
	if(getuid())
	{
		fprintf(stderr, "FtpServer must be started by root\n");
		exit(EXIT_FAILURE);
	}
}

void setup_signal_chld()
{
	if(signal(SIGCHLD, handle_sigchld) == SIG_ERR)
		ERR_EXIT("signal");
}


void limit_num_clients(session_t *sess)
{
	if(tunable_max_clients > 0 && sess->curr_clients > tunable_max_clients)
	{
		//421 There are too many connected users, please try later.
		ftp_reply(sess, FTP_TOO_MANY_USERS, "There are too many connected users, please try later.");
		exit(EXIT_FAILURE);
	}

	if(tunable_max_per_ip > 0 && sess->curr_ip_clients > tunable_max_per_ip)
	{
		//421 There are too many connections from your internet address.
		ftp_reply(sess, FTP_IP_LIMIT, "There are too many connections from your internet address.");
		exit(EXIT_FAILURE);
	}
}

static void handle_sigchld(int sig)
{
	pid_t pid;
	while((pid = waitpid(-1, NULL, WNOHANG)) > 0)
	{
		--num_of_clients;
		//pid -> ip
		uint32_t *p_ip = hash_lookup_value_by_key(pid_to_ip, &pid, sizeof(pid));
		assert(p_ip != NULL); //ip必须能找到
		uint32_t ip = *p_ip;
		//ip -> clients
		unsigned int *p_value = (unsigned int *)hash_lookup_value_by_key(ip_to_clients, &ip, sizeof(ip));
		assert(p_value != NULL && *p_value > 0);
		--*p_value;

		//释放hash表的结点
		hash_free_entry(pid_to_ip, &pid, sizeof(pid));
	}
}

//hash函数
static unsigned int hash_func(unsigned int buckets, void *key)
{
    unsigned int *number = (unsigned int*)key;

    return (*number) % buckets;
}


//在对应的ip记录中+1,返回当前ip的客户数量
static unsigned int add_ip_to_hash(uint32_t ip)
{
	unsigned int *p_value = (unsigned int *)hash_lookup_value_by_key(ip_to_clients, &ip, sizeof(ip));
	if(p_value == NULL)
	{
		unsigned int value = 1;
		hash_add_entry(ip_to_clients, &ip, sizeof(ip), &value, sizeof(value));
		return 1;
	}
	else
	{
		unsigned int value = *p_value;
		value++;
		*p_value  = value;

		return value;
	}
}


void init_hash()
{
	ip_to_clients = hash_alloc(256, hash_func);
	pid_to_ip = hash_alloc(256, hash_func);
}

void add_clients_to_hash(session_t *sess, uint32_t ip)
{
	++num_of_clients; //连接数目+1
	sess->curr_clients = num_of_clients;
	sess->curr_ip_clients = add_ip_to_hash(ip);
}

void add_pid_ip_to_hash(pid_t pid, uint32_t ip)
{
	hash_add_entry(pid_to_ip, &pid, sizeof(pid), &ip, sizeof(ip));
}
