/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

/***
 * Auto Generate by esp - nn - case
*/


#ifndef PLATE_OPT_HPP_ 
#define PLATE_OPT_HPP_

#include "network.h"
#include "operation_define.h"

#define plate_opt_blob_199_201 0
#define plate_opt_blob_202_204 1
#define plate_opt_blob_205_207 2
#define plate_opt_blob_208_210 3
#define plate_opt_blob_211_213 4
#define plate_opt_blob_214_216 5
#define plate_opt_blob_217_219 6
#define plate_opt_blob_220_222 7
#define plate_opt_blob_223_225 8
#define plate_opt_blob_226_228 9
#define plate_opt_blob_229_231 10
#define plate_opt_blob_232_234 11
#define plate_opt_blob_235_237 12
#define plate_opt_blob_238_240 13
#define plate_opt_blob_241_243 14
#define plate_opt_blob_244_246 15
#define plate_opt_blob_247_249 16
#define plate_opt_blob_250_252 17
#define plate_opt_blob_253_255 18
#define plate_opt_blob_256_258 19
#define plate_opt_blob_259_261 20
#define plate_opt_blob_262_264 21
#define plate_opt_blob_265_267 22
#define plate_opt_blob_268_270 23
#define plate_opt_blob_271_273 24
#define plate_opt_blob_274_275 25
#define plate_opt_blob_276_277 26
#define plate_opt_blob_278_279 27
#define plate_opt_blob_280_281 28
#define plate_opt_blob_282_282 29
#define plate_opt_blob_283_283 30
#define plate_opt_blob_284_285 31
#define plate_opt_blob_286_286 32
#define plate_opt_blob_287_287 33
#define plate_opt_blob_288_289 34
#define plate_opt_blob_290_290 35
#define plate_opt_blob_291_291 36
#define plate_opt_blob_292_293 37
#define plate_opt_blob_294_294 38
#define plate_opt_blob_295_295 39
#define plate_opt_blob_296_297 40
#define plate_opt_blob_298_298 41
#define plate_opt_blob_299_299 42
#define plate_opt_blob_300_301 43
#define plate_opt_blob_302_302 44
#define plate_opt_blob_303_303 45
#define plate_opt_blob_304_305 46
#define plate_opt_blob_306_306 47
#define plate_opt_blob_307_307 48
#define plate_opt_blob_308_309 49
#define plate_opt_blob_310_310 50
#define plate_opt_blob_311_311 51
#define plate_opt_blob_312_313 52
#define plate_opt_blob_314_314 53
#define plate_opt_blob_315_315 54
#define plate_opt_blob_316_316 55
#define plate_opt_blob_317_317 56
#define plate_opt_blob_318_318 57
#define plate_opt_blob_319_319 58
#define plate_opt_blob_320_320 59
#define plate_opt_blob_321_321 60
#define plate_opt_blob_331_331 61
#define plate_opt_blob_341_341 62
#define plate_opt_blob_351_351 63
#define plate_opt_blob_361_361 64
#define plate_opt_blob_372_372 65
#define plate_opt_blob_382_382 66
#define plate_opt_blob_392_392 67
#define plate_opt_blob_402_402 68
#define plate_opt_blob_403_403 69
#define plate_opt_blob_413_413 70
#define plate_opt_blob_423_423 71
#define plate_opt_blob_433_433 72
#define plate_opt_blob_443_443 73
#define plate_opt_blob_444_529 74
#define plate_opt_blob_445_530 75
#define plate_opt_blob_input0_input0 76
#define plate_opt_blob_output0_output0 77

#define PLATE_OPT_BLOB_SIZE 78

typedef struct plate_opt_ncnn_blob{ const char *name; int blob_id;} plate_opt_ncnn_blob_t;
static plate_opt_ncnn_blob_t plate_opt_ncnn_blobs [] = {
	{"input0" , plate_opt_blob_input0_input0},
	{"201" , plate_opt_blob_199_201},
	{"204" , plate_opt_blob_202_204},
	{"207" , plate_opt_blob_205_207},
	{"210" , plate_opt_blob_208_210},
	{"213" , plate_opt_blob_211_213},
	{"216" , plate_opt_blob_214_216},
	{"219" , plate_opt_blob_217_219},
	{"222" , plate_opt_blob_220_222},
	{"225" , plate_opt_blob_223_225},
	{"228" , plate_opt_blob_226_228},
	{"231" , plate_opt_blob_229_231},
	{"234" , plate_opt_blob_232_234},
	{"237" , plate_opt_blob_235_237},
	{"240" , plate_opt_blob_238_240},
	{"243" , plate_opt_blob_241_243},
	{"246" , plate_opt_blob_244_246},
	{"249" , plate_opt_blob_247_249},
	{"252" , plate_opt_blob_250_252},
	{"255" , plate_opt_blob_253_255},
	{"258" , plate_opt_blob_256_258},
	{"261" , plate_opt_blob_259_261},
	{"264" , plate_opt_blob_262_264},
	{"267" , plate_opt_blob_265_267},
	{"270" , plate_opt_blob_268_270},
	{"273" , plate_opt_blob_271_273},
	{"275" , plate_opt_blob_274_275},
	{"277" , plate_opt_blob_276_277},
	{"279" , plate_opt_blob_278_279},
	{"281" , plate_opt_blob_280_281},
	{"282" , plate_opt_blob_282_282},
	{"283" , plate_opt_blob_283_283},
	{"285" , plate_opt_blob_284_285},
	{"286" , plate_opt_blob_286_286},
	{"287" , plate_opt_blob_287_287},
	{"289" , plate_opt_blob_288_289},
	{"290" , plate_opt_blob_290_290},
	{"291" , plate_opt_blob_291_291},
	{"293" , plate_opt_blob_292_293},
	{"294" , plate_opt_blob_294_294},
	{"295" , plate_opt_blob_295_295},
	{"297" , plate_opt_blob_296_297},
	{"298" , plate_opt_blob_298_298},
	{"299" , plate_opt_blob_299_299},
	{"301" , plate_opt_blob_300_301},
	{"302" , plate_opt_blob_302_302},
	{"303" , plate_opt_blob_303_303},
	{"305" , plate_opt_blob_304_305},
	{"306" , plate_opt_blob_306_306},
	{"307" , plate_opt_blob_307_307},
	{"309" , plate_opt_blob_308_309},
	{"310" , plate_opt_blob_310_310},
	{"311" , plate_opt_blob_311_311},
	{"313" , plate_opt_blob_312_313},
	{"314" , plate_opt_blob_314_314},
	{"315" , plate_opt_blob_315_315},
	{"316" , plate_opt_blob_316_316},
	{"317" , plate_opt_blob_317_317},
	{"318" , plate_opt_blob_318_318},
	{"319" , plate_opt_blob_319_319},
	{"320" , plate_opt_blob_320_320},
	{"321" , plate_opt_blob_321_321},
	{"331" , plate_opt_blob_331_331},
	{"341" , plate_opt_blob_341_341},
	{"351" , plate_opt_blob_351_351},
	{"361" , plate_opt_blob_361_361},
	{"output0" , plate_opt_blob_output0_output0},
	{"372" , plate_opt_blob_372_372},
	{"382" , plate_opt_blob_382_382},
	{"392" , plate_opt_blob_392_392},
	{"402" , plate_opt_blob_402_402},
	{"403" , plate_opt_blob_403_403},
	{"413" , plate_opt_blob_413_413},
	{"423" , plate_opt_blob_423_423},
	{"433" , plate_opt_blob_433_433},
	{"443" , plate_opt_blob_443_443},
	{"529" , plate_opt_blob_444_529},
	{"530" , plate_opt_blob_445_530},
};

extern network_t network_plate_opt;

#endif//PLATE_OPT
