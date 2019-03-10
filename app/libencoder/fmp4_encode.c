




#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
 
#include "faac.h"
#include <math.h>



#include "typeport.h"
#include "encoder.h"
#include "fmp4_interface.h"
#include "fmp4_encode.h"

 
#define  JOSEPH_G711A_LOCATION  "/jffs0/rawaudio"
#define  JOSEPH_H264_LOCALTION "/jffs0/test0.h264"
#define  JOSEPH_MP4_FILE   "/jffs0/test.mp4"
 
#define  MP4_DETAILS_ALL     0xFFFFFFFF
#define  NALU_SPS  0
#define  NALU_PPS  1
#define  NALU_I    2
#define  NALU_P    3
#define  NALU_SET  4

#define INPUT_BUFFER_SIZE  (1024*100)   //(1048576)   //(1024*100) 

 
typedef unsigned int  uint32_t;
typedef unsigned char   uint8_t;
 
 
/********************************************************g711a encode decode**********************************************/
static const int16_t alawtos16[256] =
{
     -5504,  -5248,  -6016,  -5760,  -4480,  -4224,  -4992,  -4736,
     -7552,  -7296,  -8064,  -7808,  -6528,  -6272,  -7040,  -6784,
     -2752,  -2624,  -3008,  -2880,  -2240,  -2112,  -2496,  -2368,
     -3776,  -3648,  -4032,  -3904,  -3264,  -3136,  -3520,  -3392,
    -22016, -20992, -24064, -23040, -17920, -16896, -19968, -18944,
    -30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136,
    -11008, -10496, -12032, -11520,  -8960,  -8448,  -9984,  -9472,
    -15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568,
      -344,   -328,   -376,   -360,   -280,   -264,   -312,   -296,
      -472,   -456,   -504,   -488,   -408,   -392,   -440,   -424,
       -88,    -72,   -120,   -104,    -24,     -8,    -56,    -40,
      -216,   -200,   -248,   -232,   -152,   -136,   -184,   -168,
     -1376,  -1312,  -1504,  -1440,  -1120,  -1056,  -1248,  -1184,
     -1888,  -1824,  -2016,  -1952,  -1632,  -1568,  -1760,  -1696,
      -688,   -656,   -752,   -720,   -560,   -528,   -624,   -592,
      -944,   -912,  -1008,   -976,   -816,   -784,   -880,   -848,
      5504,   5248,   6016,   5760,   4480,   4224,   4992,   4736,
      7552,   7296,   8064,   7808,   6528,   6272,   7040,   6784,
      2752,   2624,   3008,   2880,   2240,   2112,   2496,   2368,
      3776,   3648,   4032,   3904,   3264,   3136,   3520,   3392,
     22016,  20992,  24064,  23040,  17920,  16896,  19968,  18944,
     30208,  29184,  32256,  31232,  26112,  25088,  28160,  27136,
     11008,  10496,  12032,  11520,   8960,   8448,   9984,   9472,
     15104,  14592,  16128,  15616,  13056,  12544,  14080,  13568,
       344,    328,    376,    360,    280,    264,    312,    296,
       472,    456,    504,    488,    408,    392,    440,    424,
        88,     72,    120,    104,     24,      8,     56,     40,
       216,    200,    248,    232,    152,    136,    184,    168,
      1376,   1312,   1504,   1440,   1120,   1056,   1248,   1184,
      1888,   1824,   2016,   1952,   1632,   1568,   1760,   1696,
       688,    656,    752,    720,    560,    528,    624,    592,
       944,    912,   1008,    976,    816,    784,    880,    848
};
 
static const int8_t alaw_encode[2049] =
{
    0xD5, 0xD4, 0xD7, 0xD6, 0xD1, 0xD0, 0xD3, 0xD2, 0xDD, 0xDC, 0xDF, 0xDE,
    0xD9, 0xD8, 0xDB, 0xDA, 0xC5, 0xC4, 0xC7, 0xC6, 0xC1, 0xC0, 0xC3, 0xC2,
    0xCD, 0xCC, 0xCF, 0xCE, 0xC9, 0xC8, 0xCB, 0xCA, 0xF5, 0xF5, 0xF4, 0xF4,
    0xF7, 0xF7, 0xF6, 0xF6, 0xF1, 0xF1, 0xF0, 0xF0, 0xF3, 0xF3, 0xF2, 0xF2,
    0xFD, 0xFD, 0xFC, 0xFC, 0xFF, 0xFF, 0xFE, 0xFE, 0xF9, 0xF9, 0xF8, 0xF8,
    0xFB, 0xFB, 0xFA, 0xFA, 0xE5, 0xE5, 0xE5, 0xE5, 0xE4, 0xE4, 0xE4, 0xE4,
    0xE7, 0xE7, 0xE7, 0xE7, 0xE6, 0xE6, 0xE6, 0xE6, 0xE1, 0xE1, 0xE1, 0xE1,
    0xE0, 0xE0, 0xE0, 0xE0, 0xE3, 0xE3, 0xE3, 0xE3, 0xE2, 0xE2, 0xE2, 0xE2,
    0xED, 0xED, 0xED, 0xED, 0xEC, 0xEC, 0xEC, 0xEC, 0xEF, 0xEF, 0xEF, 0xEF,
    0xEE, 0xEE, 0xEE, 0xEE, 0xE9, 0xE9, 0xE9, 0xE9, 0xE8, 0xE8, 0xE8, 0xE8,
    0xEB, 0xEB, 0xEB, 0xEB, 0xEA, 0xEA, 0xEA, 0xEA, 0x95, 0x95, 0x95, 0x95,
    0x95, 0x95, 0x95, 0x95, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94,
    0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x96, 0x96, 0x96, 0x96,
    0x96, 0x96, 0x96, 0x96, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x93, 0x93, 0x93, 0x93,
    0x93, 0x93, 0x93, 0x93, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92,
    0x9D, 0x9D, 0x9D, 0x9D, 0x9D, 0x9D, 0x9D, 0x9D, 0x9C, 0x9C, 0x9C, 0x9C,
    0x9C, 0x9C, 0x9C, 0x9C, 0x9F, 0x9F, 0x9F, 0x9F, 0x9F, 0x9F, 0x9F, 0x9F,
    0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x99, 0x99, 0x99, 0x99,
    0x99, 0x99, 0x99, 0x99, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98,
    0x9B, 0x9B, 0x9B, 0x9B, 0x9B, 0x9B, 0x9B, 0x9B, 0x9A, 0x9A, 0x9A, 0x9A,
    0x9A, 0x9A, 0x9A, 0x9A, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85,
    0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x85, 0x84, 0x84, 0x84, 0x84,
    0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84,
    0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87,
    0x87, 0x87, 0x87, 0x87, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86,
    0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
    0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x82, 0x82, 0x82, 0x82,
    0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82,
    0x8D, 0x8D, 0x8D, 0x8D, 0x8D, 0x8D, 0x8D, 0x8D, 0x8D, 0x8D, 0x8D, 0x8D,
    0x8D, 0x8D, 0x8D, 0x8D, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C,
    0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8F, 0x8F, 0x8F, 0x8F,
    0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F,
    0x8E, 0x8E, 0x8E, 0x8E, 0x8E, 0x8E, 0x8E, 0x8E, 0x8E, 0x8E, 0x8E, 0x8E,
    0x8E, 0x8E, 0x8E, 0x8E, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89,
    0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x88, 0x88, 0x88, 0x88,
    0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
    0x8B, 0x8B, 0x8B, 0x8B, 0x8B, 0x8B, 0x8B, 0x8B, 0x8B, 0x8B, 0x8B, 0x8B,
    0x8B, 0x8B, 0x8B, 0x8B, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A,
    0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0xB5, 0xB5, 0xB5, 0xB5,
    0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5,
    0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5, 0xB5,
    0xB5, 0xB5, 0xB5, 0xB5, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4,
    0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4,
    0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4,
    0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7,
    0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7,
    0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB7, 0xB6, 0xB6, 0xB6, 0xB6,
    0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6,
    0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6, 0xB6,
    0xB6, 0xB6, 0xB6, 0xB6, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1,
    0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1,
    0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1, 0xB1,
    0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0,
    0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0,
    0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB3, 0xB3, 0xB3, 0xB3,
    0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3,
    0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3, 0xB3,
    0xB3, 0xB3, 0xB3, 0xB3, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2,
    0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2,
    0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2, 0xB2,
    0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD,
    0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD,
    0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBC, 0xBC, 0xBC, 0xBC,
    0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC,
    0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC,
    0xBC, 0xBC, 0xBC, 0xBC, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF,
    0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF,
    0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF,
    0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE,
    0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE,
    0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xB9, 0xB9, 0xB9, 0xB9,
    0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9,
    0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9, 0xB9,
    0xB9, 0xB9, 0xB9, 0xB9, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8,
    0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8,
    0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8,
    0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
    0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
    0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBA, 0xBA, 0xBA, 0xBA,
    0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA,
    0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA,
    0xBA, 0xBA, 0xBA, 0xBA, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
    0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
    0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
    0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
    0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
    0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA4, 0xA4, 0xA4, 0xA4,
    0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
    0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
    0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
    0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
    0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
    0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7,
    0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7,
    0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7,
    0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7,
    0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7, 0xA7,
    0xA7, 0xA7, 0xA7, 0xA7, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6,
    0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6,
    0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6,
    0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6,
    0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6,
    0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA1, 0xA1, 0xA1, 0xA1,
    0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1,
    0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1,
    0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1,
    0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1,
    0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1, 0xA1,
    0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
    0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
    0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
    0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
    0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
    0xA0, 0xA0, 0xA0, 0xA0, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
    0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
    0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
    0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
    0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3,
    0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA3, 0xA2, 0xA2, 0xA2, 0xA2,
    0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2,
    0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2,
    0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2,
    0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2,
    0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2,
    0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD,
    0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD,
    0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD,
    0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD,
    0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD,
    0xAD, 0xAD, 0xAD, 0xAD, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
    0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
    0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
    0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
    0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
    0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAF, 0xAF, 0xAF, 0xAF,
    0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF,
    0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF,
    0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF,
    0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF,
    0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF,
    0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE,
    0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE,
    0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE,
    0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE,
    0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE,
    0xAE, 0xAE, 0xAE, 0xAE, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9,
    0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9,
    0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9,
    0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9,
    0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9,
    0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA8, 0xA8, 0xA8, 0xA8,
    0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8,
    0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8,
    0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8,
    0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8,
    0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8,
    0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB,
    0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB,
    0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB,
    0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB,
    0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB,
    0xAB, 0xAB, 0xAB, 0xAB, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x2A
}; 
 
int g711a_decode(void *pout_buf, int *pout_len, const void *pin_buf, const int  in_len)
{
    int16_t *dst = (int16_t *)pout_buf;
    uint8_t *src = (uint8_t *)pin_buf;
    uint32_t i = 0;
    int Ret = 0;
     
    if ( (NULL == pout_buf)
        || (NULL == pout_len)
        || (NULL == pin_buf)
        || (0 == in_len) )
    {
        return -1;
    }
     
    if ( *pout_len < 2 * in_len )
    {
        return -2;
    }
     
    for( i = 0; i < in_len; i++ )
    {
        *(dst++) = alawtos16[*(src++)];
    }
     
    Ret = 2 * in_len;
     
    return Ret;
}
 
int g711a_encode(void *pout_buf, int *pout_len, const void *pin_buf, const int in_len)
{
    int8_t *dst = (int8_t *)pout_buf;
    int16_t *src = (int16_t *)pin_buf;
    uint32_t i = 0;
    int Ret = 0;
     
    if ( (NULL == pout_buf)
        || (NULL == pout_len)
        || (NULL == pin_buf)
        || (0 == in_len) )
    {
        return -1;
    }
     
    if ( *pout_len < in_len / 2 )
    {
        return -2;
    }
     
    for( i = 0; i < in_len / 2; i++ )
    {
        int16_t s = *(src++);
        if( s >= 0)
        {
            *(dst++) = alaw_encode[s / 16];
        }
        else
        {
            *(dst++) = 0x7F & alaw_encode[s / -16];
        }
    }
     
    Ret = in_len / 2;
     
    return Ret;
}




/*
    判断nalu 包的类型
    返回：
        NALU_SPS  0
        NALU_PPS  1
        NALU_I    2
        NALU_P    3
        NALU_SET  4
        
*/
int nalu_type(unsigned char* naluData, int naluSize)
{
    int index = -1; 
     
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x67)
    {
        index = NALU_SPS;
        printf("%s[%d]====NALU_SPS\n",__FUNCTION__,__LINE__);
    }

    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x68)
    {
        index = NALU_PPS;
        printf("%s[%d]====NALU_PPS\n",__FUNCTION__,__LINE__);
    }
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x65)
    {
        index = NALU_I;
        printf("%s[%d]====NALU_I\n",__FUNCTION__,__LINE__);
    }
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x61)
    {
        index = NALU_P;
        printf("%s[%d]====NALU_P\n",__FUNCTION__,__LINE__);
    }
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x6)
    {
        index = NALU_SET;
        printf("%s[%d]====NALU_SET\n",__FUNCTION__,__LINE__);
    }

    return index;
}


 
//------------------------------------------------------------------------------------------------- Mp4Encode说明
// 【h264编码出的NALU规律】
// 第一帧 SPS【0 0 0 1 0x67】 PPS【0 0 0 1 0x68】 SEI【0 0 0 1 0x6】 IDR【0 0 0 1 0x65】
// p帧      P【0 0 0 1 0x61】
// I帧    SPS【0 0 0 1 0x67】 PPS【0 0 0 1 0x68】 IDR【0 0 0 1 0x65】
// 【mp4v2封装函数MP4WriteSample】
// 此函数接收I/P nalu,该nalu需要用4字节的数据大小头替换原有的起始头，并且数据大小为big-endian格式
//-------------------------------------------------------------------------------------------------
#if 0
int Mp4VEncode(fMP4_CONFIG* fmp4_config, unsigned char* naluData, int naluSize)
{
    int index = -1;
     
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x67)
    {
        index = NALU_SPS;
        printf("%s[%d]====NALU_SPS\n",__FUNCTION__,__LINE__);
    }

    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x68)
    {
        index = NALU_PPS;
        printf("%s[%d]====NALU_PPS\n",__FUNCTION__,__LINE__);
    }
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x65)
    {
        index = NALU_I;
        printf("%s[%d]====NALU_I\n",__FUNCTION__,__LINE__);
    }
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x61)
    {
        index = NALU_P;
        printf("%s[%d]====NALU_P\n",__FUNCTION__,__LINE__);
    }
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x6)
    {
        index = NALU_SET;
        printf("%s[%d]====NALU_SET\n",__FUNCTION__,__LINE__);
    }
     
    switch(index)
    {
        case NALU_SPS:         
            if(fmp4_config->video == MP4_INVALID_TRACK_ID)
            {
                fmp4_config->video = MP4AddH264VideoTrack  
                    (fmp4_config->hFile,   
                    fmp4_config->timeScale,                               //timeScale
                    (fmp4_config->timeScale / fmp4_config->fps),    //sampleDuration    timeScale/fps
                    fmp4_config->width,                                    // width  
                    fmp4_config->height,                                   // height  
                    naluData[5],                                                // sps[1] AVCProfileIndication  
                    naluData[6],                                                // sps[2] profile_compat  
                    naluData[7],                                                // sps[3] AVCLevelIndication  
                    3);                                                         // 4 bytes length before each NAL unit  
                if(fmp4_config->video == MP4_INVALID_TRACK_ID)
                {
                    printf("add video track failed.\n");
                    return -1;
                }
                //MP4SetVideoProfileLevel(fmp4_config->hFile, 1); //  Simple Profile @ Level 3   mp4编码标准
                MP4SetVideoProfileLevel(fmp4_config->hFile, 0x7F); //  Simple Profile @ Level 3
            }  
            MP4AddH264SequenceParameterSet(fmp4_config->hFile, fmp4_config->video, naluData+4, naluSize-4);  
            break;
        case NALU_PPS:  
            MP4AddH264PictureParameterSet(fmp4_config->hFile, fmp4_config->video, naluData+4, naluSize-4);  
            break;
        case NALU_SET:  
            MP4AddH264PictureParameterSet(fmp4_config->hFile, fmp4_config->video, naluData+4, naluSize-4);  
            break;
        case NALU_I: 
            {
                unsigned char* IFrameData = (unsigned char*)malloc((naluSize)* sizeof(unsigned char));

                //MP4WriteSample函数接收I/P nalu,该nalu需要用4字节的数据大小头替换原有的起始头，并且数据大小为big-endian格式
                /*大端存储模式，高地址存低位，低地址存高位*/
                IFrameData[0] = (naluSize-4) >>24;  
                IFrameData[1] = (naluSize-4) >>16;  
                IFrameData[2] = (naluSize-4) >>8;  
                IFrameData[3] = (naluSize-4) &0xff;  
                 
                memcpy(IFrameData+4, naluData+4, naluSize-4);
                //if(!MP4WriteSample(fmp4_config->hFile, fmp4_config->video, IFrameData, naluSize+1, m_vFrameDur/8000*90000, 0, 1))
                if(!MP4WriteSample(fmp4_config->hFile, fmp4_config->video, IFrameData, naluSize, MP4_INVALID_DURATION, 0, 1))
                {  
                    return -1;  
                }  
                 
                //fmp4_config->m_vFrameDur = 0;
                free(IFrameData); 
                IFrameData = NULL;
                 
                break;
            }
        case NALU_P:
            {
                //MP4WriteSample函数接收I/P nalu,该nalu需要用4字节的数据大小头替换原有的起始头，并且数据大小为big-endian格式
                /*大端存储模式，高地址存低位，低地址存高位*/
                naluData[0] = (naluSize-4) >>24;  
                naluData[1] = (naluSize-4) >>16;  
                naluData[2] = (naluSize-4) >>8;  
                naluData[3] = (naluSize-4) &0xff; 
                 
                //if(!MP4WriteSample(fmp4_config->hFile, fmp4_config->video, naluData, naluSize, m_vFrameDur/8000*90000, 0, 1))
                if(!MP4WriteSample(fmp4_config->hFile, fmp4_config->video, naluData, naluSize, MP4_INVALID_DURATION, 0, 1))
                {  
                    return -1;  
                }
                 
                //fmp4_config->m_vFrameDur = 0;
                 
                break;
            }
        default:
            break;
    }
         
    return 0;
}

#endif



//用来以十六进制字节流打印box
void print_array(unsigned char* box_name,unsigned char*start,unsigned int length)
{
    unsigned char*p = start;
    unsigned int i = 0;
    printf("\n  %s[]: ",box_name);
    for(i=0;i<length;i++)
    {
        printf("%x ",p[i]);
    }
    printf("\n");

}


/*********************************************main**************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define RECODE_STREAM_ID 0     //进行录像的流id       ： 0或者1
#define VIDEO_RECORD_TIME 15   //录像时长 单位：S    
#define FIND_IDR_MAX_NUM  200  //为了寻找IDR的的最大循环次数
//#define FMP4_FILE  "/jffs0/fmp4.mp4"
#define FMP4_FILE  "/ramfs/fmp4.mp4"

#define AUDIO_SAMPLING_RATE (16000) //音频的原始采样率（PCM）
#define V_FRAME_RATE  (15)  //视频帧帧率
/*-----------------------------------------------------------------
1.使用 EasyAACEncoder库: 
    44.1KHZ:A_FRAME_RATE = 15 帧/秒
    8KHZ ： A_FRAME_RATE  = 8  帧/秒 
2.使用 faac库：
    44.1KHZ A_FRAME_RATE = 27 帧/秒 (数据帧较EasyAACEncoder的小很多)
    16KHZ   A_FRAME_RATE = 14 帧/秒 (数据帧较EasyAACEncoder的小很多)
注意:
    实测，在16KHZ、44.1KHZ下使用EasyAACEncoder库编码出来的音频有
    倍速效果（不推荐），但8KHZ下比较正常。
----------------------------------------------------------------*/
#define A_FRAME_RATE  (14)  //aac音频数据帧率

#define OUT_FILE_BUF_SIZE (4*1024*1024) //初始化 3M大小空间（30S音视频）

extern HLE_S32  connected_client_num;
extern int  client_stream_id;

void* fmp4_record(void* args)
{

    pthread_detach(pthread_self()); 
    DEBUG_LOG("start fmp4_record..... \n");  

    sleep(5); //如若刚开机，不要太快开始录像
    /*init fmp4 encoder*/
    fmp4_out_info_t info = {0};
    info.buf_mode.buf_size = OUT_FILE_BUF_SIZE; 
    info.buf_mode.buf_start = (unsigned char*)malloc(OUT_FILE_BUF_SIZE);
    if(NULL ==  info.buf_mode.buf_start)
    {
        ERROR_LOG("malloc failed !\n");
        return NULL;
    }
    info.buf_mode.w_offset = 0;
    int ret = Fmp4_encode_init(&info,V_FRAME_RATE , A_FRAME_RATE , AUDIO_SAMPLING_RATE);
    if(ret < 0)
    {
        ERROR_LOG("fmp4 encode init failed!\n");
        goto InitFmp4Encoder_Failed;
        
    }
        
    unsigned int skip_len = 0;
    unsigned int frame_len = 0;
    struct timeval start_record_time = {0};
    struct timeval tmp_time = {0};
    ENC_STREAM_PACK *pack = NULL;
    FRAME_HDR *header  = NULL;
    int  stream_id;
   
    //--request stream--------------------------------------
    stream_id = encoder_request_stream(0, RECODE_STREAM_ID, 1);
    
    while(1)  //保证第一帧是视频关键帧
    {
        printf("search to IDR frame...\n");
        pack = encoder_get_packet(stream_id);
        header = (FRAME_HDR *) pack->data;
        
        if(header->type != 0xF8)//寻找IDR帧
        {
            printf("release pack ...");
            encoder_release_packet(pack);
            continue;
        }
        else
        {
            printf("IDR frame finded*******!\n");
            break;
        }       
    }
 
    
    //---debug 部分-------------------------------------------------------------------------
    #if 0   //保存找到的第一帧I帧数据到文件
        int tmp_fd = open("/jffs0/IDR_NALU.bin",O_RDWR|O_CREAT|O_TRUNC,0777);
        if(tmp_fd < 0)
        {
            ERROR_LOG("open failed !\n");
            goto other_error;
        }

        skip_len = sizeof (FRAME_HDR) + sizeof (IFRAME_INFO);
        frame_len = pack->length - skip_len;
            
        int tmp_ret = write(tmp_fd, pack->data + skip_len ,frame_len);
        if(tmp_ret != frame_len)
        {
            ERROR_LOG("write error!\n");
            close(tmp_fd);
            goto other_error;
        }
        close(tmp_fd);
    #endif

    gettimeofday(&start_record_time,NULL);
    memcpy(&tmp_time,&start_record_time,sizeof(start_record_time));
    printf("seconds:%ld  microseconds:%ld\n",start_record_time.tv_sec,start_record_time.tv_usec);
    int cucle_num = 0;
    unsigned int A_count = 0;
    unsigned int V_count = 0;
    
    IFRAME_INFO * start_info = (IFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
    unsigned long long int start_time = start_info->pts_msec;
    unsigned long long int cur_time = start_time;
     struct timeval audio_time_start = {0};
     struct timeval video_time_start = {0};
     struct timeval audio_time_current = {0};
     struct timeval video_time_current = {0};
     gettimeofday(&audio_time_start,NULL);
     gettimeofday(&video_time_start,NULL);
   // while(tmp_time.tv_sec - start_record_time.tv_sec < VIDEO_RECORD_TIME)
   while(1)//录制的时长用视频帧的帧头时间来进行计时比较准确
    {
        //找到IDR帧，开始录制
         gettimeofday(&audio_time_current,NULL);
        if(audio_time_current.tv_sec - audio_time_start.tv_sec >=1)//1s 打印一次
        {
            printf("count Aframe = %d\n",A_count);
            memcpy(&audio_time_start,&audio_time_current,sizeof(struct timeval));
            A_count = 0;
        }
         gettimeofday(&video_time_current,NULL);
        if(video_time_current.tv_sec - video_time_start.tv_sec >=1)//1s 打印一次
        {
            printf("count Vframe = %d\n",V_count);
            memcpy(&video_time_start,&video_time_current,sizeof(struct timeval));
            V_count = 0;
        }
         if (header->type == 0xFA)//0xFA-音频帧
        {
            A_count ++;
            #if 1
                skip_len = sizeof (FRAME_HDR) + sizeof (AFRAME_INFO);
                frame_len = pack->length - skip_len; 
                AFRAME_INFO * A_info = (AFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
                // printf("Audio pts : %lld\n",A_info->pts_msec);
                    
                if(Fmp4AEncode((unsigned char*)pack->data + skip_len,frame_len, A_FRAME_RATE,A_info->pts_msec))
                {
                    ERROR_LOG("Fmp4AEncode failed !\n");
                    goto other_error;
                }
            
            #endif

        }
        else if(header->type == 0xF8)   //0xF8-视频关键帧 
        {
            V_count ++;
            // printf("I frame ========\n");
            #if 1
                skip_len = sizeof (FRAME_HDR) + sizeof (IFRAME_INFO);
                frame_len = pack->length - skip_len;    
                IFRAME_INFO * V_info = (IFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
                cur_time = V_info->pts_msec;
                if(Fmp4VEncode((unsigned char*)pack->data + skip_len,frame_len,V_FRAME_RATE,V_info->pts_msec))
                {
                    ERROR_LOG("Fmp4VEncode failed !\n");
                    goto other_error;
                }
            
            #endif

        }
        else if(header->type == 0xF9)//0xF9-视频非关键帧
        {
            V_count ++;
           // printf("P frame\n");
            #if 1
                skip_len = sizeof (FRAME_HDR) + sizeof (PFRAME_INFO);
                frame_len = pack->length - skip_len;    
                PFRAME_INFO * V_info = (PFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
                cur_time = V_info->pts_msec;
                if(Fmp4VEncode((unsigned char*)pack->data + skip_len,frame_len,V_FRAME_RATE,V_info->pts_msec))
                {
                    ERROR_LOG("Fmp4VEncode failed !\n");
                    goto other_error;
                }
            
            #endif
        }
        else
        {
            ERROR_LOG("unknown frame type!\n");
        }

        encoder_release_packet(pack);
        GET_PACK:
        pack = encoder_get_packet(stream_id);
        header = (FRAME_HDR *) pack->data;

        if(cur_time - start_time >= VIDEO_RECORD_TIME*1000) break; //录制完成
            
        
    }

    DEBUG_LOG("End of recording time!\n");

other_error:
    encoder_free_stream(stream_id);
 
InitFmp4Encoder_Failed:
    //encode part
    Fmp4_encode_exit();
    free(info.buf_mode.buf_start);
    info.buf_mode.buf_start = NULL;
    DEBUG_LOG("The fmp4_record exit !\n");  

    return NULL;
}























































