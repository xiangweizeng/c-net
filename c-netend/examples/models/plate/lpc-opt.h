/***
 * Auto Generate by esp - nn - case
*/


#ifndef LPC_OPT_HPP_ 
#define LPC_OPT_HPP_

#include "network.h"
#include "operation_define.h"

#define lpc_opt_blob__reshape_ip2__reshape_ip2 0
#define lpc_opt_blob_conv1_conv1 1
#define lpc_opt_blob_conv2_conv2 2
#define lpc_opt_blob_conv3_conv3 3
#define lpc_opt_blob_conv4_conv4 4
#define lpc_opt_blob_data_data 5
#define lpc_opt_blob_ip2_ip2 6
#define lpc_opt_blob_pool1_pool1 7
#define lpc_opt_blob_pool2_pool2 8
#define lpc_opt_blob_pool3_pool3 9
#define lpc_opt_blob_pool4_pool4 10
#define lpc_opt_blob_prob_prob 11
#define lpc_opt_blob_relu1_relu1 12
#define lpc_opt_blob_relu2_relu2 13
#define lpc_opt_blob_relu3_relu3 14
#define lpc_opt_blob_relu4_relu4 15

#define LPC_OPT_BLOB_SIZE 16

typedef struct lpc_opt_ncnn_blob{ const char *name; int blob_id;} lpc_opt_ncnn_blob_t;
static lpc_opt_ncnn_blob_t lpc_opt_ncnn_blobs [] = {
	{"data" , lpc_opt_blob_data_data},
	{"conv1" , lpc_opt_blob_conv1_conv1},
	{"pool1" , lpc_opt_blob_pool1_pool1},
	{"relu1" , lpc_opt_blob_relu1_relu1},
	{"conv2" , lpc_opt_blob_conv2_conv2},
	{"relu2" , lpc_opt_blob_relu2_relu2},
	{"pool2" , lpc_opt_blob_pool2_pool2},
	{"conv3" , lpc_opt_blob_conv3_conv3},
	{"relu3" , lpc_opt_blob_relu3_relu3},
	{"pool3" , lpc_opt_blob_pool3_pool3},
	{"conv4" , lpc_opt_blob_conv4_conv4},
	{"relu4" , lpc_opt_blob_relu4_relu4},
	{"pool4" , lpc_opt_blob_pool4_pool4},
	{"_reshape_ip2" , lpc_opt_blob__reshape_ip2__reshape_ip2},
	{"ip2" , lpc_opt_blob_ip2_ip2},
	{"prob" , lpc_opt_blob_prob_prob},
};

extern network_t network_lpc_opt;

#endif//LPC_OPT
