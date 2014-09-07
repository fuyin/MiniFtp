#include "trans_ctrl.h"
#include "sysutil.h"
#include "configure.h"
#include "common.h"
#include "command_map.h"
#include "ftp_code.h"

session_t *p_sess =NULL;
void cancel_signal_fd();
static void handle_signal_data_fd(int sig);
static void handle_signal_ctrl_fd(int sig);

void limit_curr_rate(session_t *sess,int nbytes,int is_up)
{
    int curr_time_sec= get_curr_time_sec();
    int curr_time_usec = get_curr_time_usec();
    //求时间差
    double elapsed = 0.0;
    elapsed += (curr_time_sec  -sess->start_time_sec);
    elapsed += (curr_time_usec - sess->start_time_usec)/(double)1000000;

    if(elapsed < 0.0000001)
        elapsed = 0.001;

    //rate
    double rate = nbytes/elapsed;

    //radio
    double rate_radio =0.0;

    if(is_up==1)
    {
        if(sess->limits_max_upload>0 && sess->limits_max_upload < rate)
        {
            rate_radio = rate / sess->limits_max_upload;
        }
        else
        {
            //如果不限速，必须更新时间
            sess->start_time_sec=get_curr_time_sec();
            sess->start_time_usec= get_curr_time_usec();
            return;
        }
    }
    else
    {
        if(sess->limits_max_download>0 && sess->limits_max_download < rate)
        {
            rate_radio = rate / sess->limits_max_download;
        }
        else
        {
            //如果不限速，必须更新时间
            sess->start_time_sec=get_curr_time_sec();
            sess->start_time_usec= get_curr_time_usec();
            return;
        }
        
    }
    double nano_time= (rate_radio-1)*elapsed;

    if(nano_sleep(nano_time) == -1)
        ERR_EXIT("nano_sleep");

            //必须更新时间
            sess->start_time_sec=get_curr_time_sec();
            sess->start_time_usec= get_curr_time_usec();
    
}
void setup_signal_alarm_ctrl_fd()
{
    if(signal(SIGALRM,handle_signal_ctrl_fd)==SIG_ERR)
        ERR_EXIT("signal");
}

void start_signal_alarm_ctrl_fd()
{
    alarm(tunable_idle_session_timeout);
}

void setup_signal_alarm_data_fd()
{
    if(signal(SIGALRM,handle_signal_data_fd)==SIG_ERR)
        ERR_EXIT("signal");
}

void start_signal_alarm_data_fd()
{
    alarm(tunable_connect_timeout);
}

static void handle_signal_ctrl_fd(int sig)
{
    if(tunable_idle_session_timeout >0)
    {
        shutdown(p_sess->peerfd,SHUT_RD);
        ftp_reply(p_sess,FTP_IDLE_TIMEOUT,"Timeout.");
        shutdown(p_sess->peerfd,SHUT_WR);
        exit(EXIT_SUCCESS);
    }
}

static void handle_signal_data_fd(int sig)
{
    if(tunable_connect_timeout > 0)
    {
        if(p_sess ->is_translating_data ==1)
        {
            start_signal_alarm_data_fd();
        }
        else
        {
            close(p_sess->data_fd);
        shutdown(p_sess->peerfd,SHUT_RD);
        ftp_reply(p_sess,FTP_IDLE_TIMEOUT,"Timeout.");
        shutdown(p_sess->peerfd,SHUT_WR);
        exit(EXIT_SUCCESS);
        }
    }
}

 void cancel_signal_fd()
{
    alarm(0);
}
