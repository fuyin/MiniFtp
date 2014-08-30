#include "parse_conf.h"
#include "common.h"
#include "strutil.h"
#include "configure.h"

void parseconf_load_setting(const char *setting);

//boolÀÐµÄä±ä
static struct parseconf_bool_setting
{
  const char *p_setting_name;
  int *p_variable;
}
parseconf_bool_array[] =
{
    { "pasv_enable", &tunable_pasv_enable },
    { "port_enable", &tunable_port_enable },
    { NULL, NULL }
};

static struct parseconf_uint_setting
{
    const char *p_setting_name;
    unsigned int *p_variable;
}
parseconf_uint_array[] =
{
    { "listen_port", &tunable_listen_port },
    { "max_clients", &tunable_max_clients },
    { "max_per_ip", &tunable_max_per_ip },
    { "accept_timeout", &tunable_accept_timeout },
    { "connect_timeout", &tunable_connect_timeout },
    { "idle_session_timeout", &tunable_idle_session_timeout },
    { "data_connection_timeout", &tunable_data_connection_timeout },
    { "local_umask", &tunable_local_umask },
    { "upload_max_rate", &tunable_upload_max_rate },
    { "download_max_rate", &tunable_download_max_rate },
    { NULL, NULL }
};

static struct parseconf_str_setting
{
    const char *p_setting_name;
    const char **p_variable;
}
parseconf_str_array[] =
{
    { "listen_address", &tunable_listen_address },
    { NULL, NULL }
};


void parse_conf_load_file(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        ERR_EXIT("fopen");

    char setting_line[1024] = {0};
    while (fgets(setting_line, sizeof(setting_line), fp) != NULL)
    {
        if (strlen(setting_line) == 0
            || setting_line[0] == '#'
            || str_all_space(setting_line))
            continue;

        str_trim_crlf(setting_line);
        parseconf_load_setting(setting_line);
        memset(setting_line, 0, sizeof(setting_line));
    }

    fclose(fp);
}

int isSpace(char c)
{
    if(c==' ')
    return 1;
    else
        return 0;
}
void parseconf_load_setting(const char *setting)
{
    // ȥ³ýñh
    while(isSpace(*setting))
        setting++;
    char key[128] ={0};
    char value[128] = {0};
    str_split(setting, key, value, '=');
    if (strlen(value) == 0)
    {
        fprintf(stderr, "mising value in config file for: %s\n", key);
        exit(EXIT_FAILURE);
    }
    {
        const struct parseconf_str_setting *p_str_setting = parseconf_str_array;
        while (p_str_setting->p_setting_name != NULL)
        {
            if (strcmp(key, p_str_setting->p_setting_name) == 0)
            {
                const char **p_cur_setting = p_str_setting->p_variable;
                if (*p_cur_setting)
                    free((char*)*p_cur_setting);

                *p_cur_setting = strdup(value);
                return;
            }

            p_str_setting++;
        }
    }

    {
        const struct parseconf_bool_setting *p_bool_setting = parseconf_bool_array;
        while (p_bool_setting->p_setting_name != NULL)
        {
            if (strcmp(key, p_bool_setting->p_setting_name) == 0)
            {
                str_upper(value);
                if (strcmp(value, "YES") == 0
                    || strcmp(value, "TRUE") == 0
                    || strcmp(value, "1") == 0)
                    *(p_bool_setting->p_variable) = 1;
                else if (strcmp(value, "NO") == 0
                    || strcmp(value, "FALSE") == 0
                    || strcmp(value, "0") == 0)
                    *(p_bool_setting->p_variable) = 0;
                else
                {
                    fprintf(stderr, "bad bool value in config file for: %s\n", key);
                    exit(EXIT_FAILURE);
                }

                return;
            }

            p_bool_setting++;
        }
    }

    {
        const struct parseconf_uint_setting *p_uint_setting = parseconf_uint_array;
        while (p_uint_setting->p_setting_name != NULL)
        {
            if (strcmp(key, p_uint_setting->p_setting_name) == 0)
            {
                if (value[0] == '0')
                    *(p_uint_setting->p_variable) = str_octal_to_uint(value);
                else
                    *(p_uint_setting->p_variable) = atoi(value);

                return;
            }

            p_uint_setting++;
        }
    }
}
void print_conf()
{
    printf("tunable_pasv_enable=%d\n", tunable_pasv_enable);
    printf("tunable_port_enable=%d\n", tunable_port_enable);

    printf("tunable_listen_port=%u\n", tunable_listen_port);
    printf("tunable_max_clients=%u\n", tunable_max_clients);
    printf("tunable_max_per_ip=%u\n", tunable_max_per_ip);
    printf("tunable_accept_timeout=%u\n", tunable_accept_timeout);
    printf("tunable_connect_timeout=%u\n", tunable_connect_timeout);
    printf("tunable_idle_session_timeout=%u\n", tunable_idle_session_timeout);
    printf("tunable_data_connection_timeout=%u\n", tunable_data_connection_timeout);
    printf("tunable_local_umask=0%o\n", tunable_local_umask);
    printf("tunable_upload_max_rate=%u\n", tunable_upload_max_rate);
    printf("tunable_download_max_rate=%u\n", tunable_download_max_rate);

    if (tunable_listen_address == NULL)
        printf("tunable_listen_address=NULL\n");
    else
        printf("tunable_listen_address=%s\n", tunable_listen_address); 
}
