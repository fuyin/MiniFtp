#ifndef TRANS_CTRL_H
#define TRANS_CTRL_H 
#include "session.h"

void limit_curr_rate(session_t *sess, int nbytes,int is_up);

void setup_signal_alarm_ctrl_fd();
void start_signal_alarm_ctrl_fd();
void setup_signal_alarm_data_fd();
void start_signal_alarm_data_fd();
void cancel_signal_fd();

#endif  /*TRANS_CTRL_H*/
