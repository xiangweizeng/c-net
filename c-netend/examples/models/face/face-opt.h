/***
 * Auto Generate by esp - nn - case
*/


#ifndef FACE_OPT_HPP_ 
#define FACE_OPT_HPP_

#include "network.h"
#include "operation_define.h"

#define face_opt_blob_259_261 0
#define face_opt_blob_262_264 1
#define face_opt_blob_265_267 2
#define face_opt_blob_268_270 3
#define face_opt_blob_271_273 4
#define face_opt_blob_274_276 5
#define face_opt_blob_277_279 6
#define face_opt_blob_280_282 7
#define face_opt_blob_283_285 8
#define face_opt_blob_286_288 9
#define face_opt_blob_289_291 10
#define face_opt_blob_292_294 11
#define face_opt_blob_295_297 12
#define face_opt_blob_298_298 13
#define face_opt_blob_300_302 14
#define face_opt_blob_303_303 15
#define face_opt_blob_305_305 16
#define face_opt_blob_307_309 17
#define face_opt_blob_310_310 18
#define face_opt_blob_312_312 19
#define face_opt_blob_314_316 20
#define face_opt_blob_317_319 21
#define face_opt_blob_320_320 22
#define face_opt_blob_322_322 23
#define face_opt_blob_323_323 24
#define face_opt_blob_325_325 25
#define face_opt_blob_327_327 26
#define face_opt_blob_328_328 27
#define face_opt_blob_329_331 28
#define face_opt_blob_332_334 29
#define face_opt_blob_335_337 30
#define face_opt_blob_338_340 31
#define face_opt_blob_341_343 32
#define face_opt_blob_344_346 33
#define face_opt_blob_347_349 34
#define face_opt_blob_350_352 35
#define face_opt_blob_353_355 36
#define face_opt_blob_356_358 37
#define face_opt_blob_359_360 38
#define face_opt_blob_361_362 39
#define face_opt_blob_363_364 40
#define face_opt_blob_365_366 41
#define face_opt_blob_367_367 42
#define face_opt_blob_369_370 43
#define face_opt_blob_371_371 44
#define face_opt_blob_373_374 45
#define face_opt_blob_375_375 46
#define face_opt_blob_377_378 47
#define face_opt_blob_379_379 48
#define face_opt_blob_381_382 49
#define face_opt_blob_383_383 50
#define face_opt_blob_385_386 51
#define face_opt_blob_387_387 52
#define face_opt_blob_389_390 53
#define face_opt_blob_391_391 54
#define face_opt_blob_393_394 55
#define face_opt_blob_395_395 56
#define face_opt_blob_397_398 57
#define face_opt_blob_399_399 58
#define face_opt_blob_401_401 59
#define face_opt_blob_403_403 60
#define face_opt_blob_405_405 61
#define face_opt_blob_416_416 62
#define face_opt_blob_426_426 63
#define face_opt_blob_436_436 64
#define face_opt_blob_446_446 65
#define face_opt_blob_457_457 66
#define face_opt_blob_467_467 67
#define face_opt_blob_477_477 68
#define face_opt_blob_487_487 69
#define face_opt_blob_488_488 70
#define face_opt_blob_498_498 71
#define face_opt_blob_508_508 72
#define face_opt_blob_518_518 73
#define face_opt_blob_528_528 74
#define face_opt_blob_529_529 75
#define face_opt_blob_530_530 76
#define face_opt_blob_input0_input0 77
#define face_opt_blob_output0_output0 78

#define FACE_OPT_BLOB_SIZE 79

typedef struct face_opt_ncnn_blob{ const char *name; int blob_id;} face_opt_ncnn_blob_t;
static face_opt_ncnn_blob_t face_opt_ncnn_blobs [] = {
	{"input0" , face_opt_blob_input0_input0},
	{"261" , face_opt_blob_259_261},
	{"264" , face_opt_blob_262_264},
	{"267" , face_opt_blob_265_267},
	{"270" , face_opt_blob_268_270},
	{"273" , face_opt_blob_271_273},
	{"276" , face_opt_blob_274_276},
	{"279" , face_opt_blob_277_279},
	{"282" , face_opt_blob_280_282},
	{"285" , face_opt_blob_283_285},
	{"288" , face_opt_blob_286_288},
	{"291" , face_opt_blob_289_291},
	{"294" , face_opt_blob_292_294},
	{"297" , face_opt_blob_295_297},
	{"298" , face_opt_blob_298_298},
	{"302" , face_opt_blob_300_302},
	{"303" , face_opt_blob_303_303},
	{"305" , face_opt_blob_305_305},
	{"309" , face_opt_blob_307_309},
	{"310" , face_opt_blob_310_310},
	{"312" , face_opt_blob_312_312},
	{"316" , face_opt_blob_314_316},
	{"319" , face_opt_blob_317_319},
	{"320" , face_opt_blob_320_320},
	{"322" , face_opt_blob_322_322},
	{"323" , face_opt_blob_323_323},
	{"325" , face_opt_blob_325_325},
	{"327" , face_opt_blob_327_327},
	{"328" , face_opt_blob_328_328},
	{"331" , face_opt_blob_329_331},
	{"334" , face_opt_blob_332_334},
	{"337" , face_opt_blob_335_337},
	{"340" , face_opt_blob_338_340},
	{"343" , face_opt_blob_341_343},
	{"346" , face_opt_blob_344_346},
	{"349" , face_opt_blob_347_349},
	{"352" , face_opt_blob_350_352},
	{"355" , face_opt_blob_353_355},
	{"358" , face_opt_blob_356_358},
	{"360" , face_opt_blob_359_360},
	{"362" , face_opt_blob_361_362},
	{"364" , face_opt_blob_363_364},
	{"366" , face_opt_blob_365_366},
	{"367" , face_opt_blob_367_367},
	{"370" , face_opt_blob_369_370},
	{"371" , face_opt_blob_371_371},
	{"374" , face_opt_blob_373_374},
	{"375" , face_opt_blob_375_375},
	{"378" , face_opt_blob_377_378},
	{"379" , face_opt_blob_379_379},
	{"382" , face_opt_blob_381_382},
	{"383" , face_opt_blob_383_383},
	{"386" , face_opt_blob_385_386},
	{"387" , face_opt_blob_387_387},
	{"390" , face_opt_blob_389_390},
	{"391" , face_opt_blob_391_391},
	{"394" , face_opt_blob_393_394},
	{"395" , face_opt_blob_395_395},
	{"398" , face_opt_blob_397_398},
	{"399" , face_opt_blob_399_399},
	{"401" , face_opt_blob_401_401},
	{"403" , face_opt_blob_403_403},
	{"405" , face_opt_blob_405_405},
	{"416" , face_opt_blob_416_416},
	{"426" , face_opt_blob_426_426},
	{"436" , face_opt_blob_436_436},
	{"446" , face_opt_blob_446_446},
	{"output0" , face_opt_blob_output0_output0},
	{"457" , face_opt_blob_457_457},
	{"467" , face_opt_blob_467_467},
	{"477" , face_opt_blob_477_477},
	{"487" , face_opt_blob_487_487},
	{"488" , face_opt_blob_488_488},
	{"498" , face_opt_blob_498_498},
	{"508" , face_opt_blob_508_508},
	{"518" , face_opt_blob_518_518},
	{"528" , face_opt_blob_528_528},
	{"529" , face_opt_blob_529_529},
	{"530" , face_opt_blob_530_530},
};

extern network_t network_face_opt;

#endif//FACE_OPT
