/***
 * Auto Generate by esp - nn - case
*/


#ifndef LPR_OPT_HPP_ 
#define LPR_OPT_HPP_

#include "network.h"
#include "operation_define.h"

#define lpr_opt_blob__permute1_reshape_fc1x__permute1_reshape_fc1x 0
#define lpr_opt_blob__reshape_fc1x__reshape_fc1x 1
#define lpr_opt_blob_conv1_conv1_relu 2
#define lpr_opt_blob_fc1x__permute2_reshape_fc1x 3
#define lpr_opt_blob_input_data 4
#define lpr_opt_blob_pool1_pool1 5
#define lpr_opt_blob_pool5_ave_pool5_ave 6
#define lpr_opt_blob_pool5_ave_transpose_pool5_ave_transpose 7
#define lpr_opt_blob_res2a_branch1_scale2a_branch1 8
#define lpr_opt_blob_res2a_branch2a_res2a_branch2a_relu 9
#define lpr_opt_blob_res2a_branch2b_scale2a_branch2b 10
#define lpr_opt_blob_res2a_relu_res2a_relu 11
#define lpr_opt_blob_res2a_res2a 12
#define lpr_opt_blob_res2b_branch2a_res2b_branch2a_relu 13
#define lpr_opt_blob_res2b_branch2b_scale2b_branch2b 14
#define lpr_opt_blob_res2b_relu_res2b_relu 15
#define lpr_opt_blob_res2b_res2b 16
#define lpr_opt_blob_res3a_branch1_scale3a_branch1 17
#define lpr_opt_blob_res3a_branch2a_res3a_branch2a_relu 18
#define lpr_opt_blob_res3a_branch2b_scale3a_branch2b 19
#define lpr_opt_blob_res3a_relu_res3a_relu 20
#define lpr_opt_blob_res3a_res3a 21
#define lpr_opt_blob_res3b_branch2a_res3b_branch2a_relu 22
#define lpr_opt_blob_res3b_branch2b_scale3b_branch2b 23
#define lpr_opt_blob_res3b_relu_res3b_relu 24
#define lpr_opt_blob_res3b_res3b 25
#define lpr_opt_blob_res4a_branch1_scale4a_branch1 26
#define lpr_opt_blob_res4a_branch2a_res4a_branch2a_relu 27
#define lpr_opt_blob_res4a_branch2b_scale4a_branch2b 28
#define lpr_opt_blob_res4a_relu_res4a_relu 29
#define lpr_opt_blob_res4a_res4a 30
#define lpr_opt_blob_res4b_branch2a_res4b_branch2a_relu 31
#define lpr_opt_blob_res4b_branch2b_scale4b_branch2b 32
#define lpr_opt_blob_res4b_relu_res4b_relu 33
#define lpr_opt_blob_res4b_res4b 34

#define LPR_OPT_BLOB_SIZE 35

typedef struct lpr_opt_ncnn_blob{ const char *name; int blob_id;} lpr_opt_ncnn_blob_t;
static lpr_opt_ncnn_blob_t lpr_opt_ncnn_blobs [] = {
	{"data" , lpr_opt_blob_input_data},
	{"conv1_relu" , lpr_opt_blob_conv1_conv1_relu},
	{"pool1" , lpr_opt_blob_pool1_pool1},
	{"scale2a_branch1" , lpr_opt_blob_res2a_branch1_scale2a_branch1},
	{"res2a_branch2a_relu" , lpr_opt_blob_res2a_branch2a_res2a_branch2a_relu},
	{"scale2a_branch2b" , lpr_opt_blob_res2a_branch2b_scale2a_branch2b},
	{"res2a" , lpr_opt_blob_res2a_res2a},
	{"res2a_relu" , lpr_opt_blob_res2a_relu_res2a_relu},
	{"res2b_branch2a_relu" , lpr_opt_blob_res2b_branch2a_res2b_branch2a_relu},
	{"scale2b_branch2b" , lpr_opt_blob_res2b_branch2b_scale2b_branch2b},
	{"res2b" , lpr_opt_blob_res2b_res2b},
	{"res2b_relu" , lpr_opt_blob_res2b_relu_res2b_relu},
	{"scale3a_branch1" , lpr_opt_blob_res3a_branch1_scale3a_branch1},
	{"res3a_branch2a_relu" , lpr_opt_blob_res3a_branch2a_res3a_branch2a_relu},
	{"scale3a_branch2b" , lpr_opt_blob_res3a_branch2b_scale3a_branch2b},
	{"res3a" , lpr_opt_blob_res3a_res3a},
	{"res3a_relu" , lpr_opt_blob_res3a_relu_res3a_relu},
	{"res3b_branch2a_relu" , lpr_opt_blob_res3b_branch2a_res3b_branch2a_relu},
	{"scale3b_branch2b" , lpr_opt_blob_res3b_branch2b_scale3b_branch2b},
	{"res3b" , lpr_opt_blob_res3b_res3b},
	{"res3b_relu" , lpr_opt_blob_res3b_relu_res3b_relu},
	{"scale4a_branch1" , lpr_opt_blob_res4a_branch1_scale4a_branch1},
	{"res4a_branch2a_relu" , lpr_opt_blob_res4a_branch2a_res4a_branch2a_relu},
	{"scale4a_branch2b" , lpr_opt_blob_res4a_branch2b_scale4a_branch2b},
	{"res4a" , lpr_opt_blob_res4a_res4a},
	{"res4a_relu" , lpr_opt_blob_res4a_relu_res4a_relu},
	{"res4b_branch2a_relu" , lpr_opt_blob_res4b_branch2a_res4b_branch2a_relu},
	{"scale4b_branch2b" , lpr_opt_blob_res4b_branch2b_scale4b_branch2b},
	{"res4b" , lpr_opt_blob_res4b_res4b},
	{"res4b_relu" , lpr_opt_blob_res4b_relu_res4b_relu},
	{"pool5_ave" , lpr_opt_blob_pool5_ave_pool5_ave},
	{"pool5_ave_transpose" , lpr_opt_blob_pool5_ave_transpose_pool5_ave_transpose},
	{"_reshape_fc1x" , lpr_opt_blob__reshape_fc1x__reshape_fc1x},
	{"_permute1_reshape_fc1x" , lpr_opt_blob__permute1_reshape_fc1x__permute1_reshape_fc1x},
	{"_permute2_reshape_fc1x" , lpr_opt_blob_fc1x__permute2_reshape_fc1x},
};

extern network_t network_lpr_opt;

#endif//LPR_OPT
