#ifndef TRANS_DATA_H
#define TRANS_DATA_H
#include "session.h"
void trans_list(session_t *sess);
void trans_list_simple(session_t *sess);
int get_trans_data_fd(session_t *sess);
void upload_file(session_t *sess,int is_appe);

#endif  /*TRANS_DATA_H*/
