/************************************************************
*
* In diesem Modul werden die Tastaturcodes der Sondertasten
* Festgelegt. Unter OS/2 kann jede Taste mittels eines
* short int's gestgelegt werden, wÑhrend unter DOS hierfÅr
* zwei aufeinanderfolgende Zeichen notwendig sind, von
* denen das erste \0 ist.
* FÅr DOS nehmen die hier definierten Konstanten den
* Wert des zweiten (auf \0 folgenden) Zeichens an.
*
************************************************************/

#ifndef _KEYS_DEFINED_
#define _KEYS_DEFINED_
#define END_KEY  '\r'           /* Defines fuer Windowoperationen */
#ifdef OS2
#define KEY_RIGHT 19712
#define KEY_LEFT  19200
#define KEY_UP    18432
#define KEY_DOWN  20480
#define KEY_WORDR 29696
#define KEY_WORDL 29440
#define KEY_DEL   21248
#define KEY_HOME  18176
#define KEY_END   20224
#define KEY_INS   20992
#define KEY_DC    21248
#else
#define KEY_RIGHT 77
#define KEY_LEFT  75
#define KEY_UP    72
#define KEY_DOWN  80
#define KEY_WORDR 116
#define KEY_WORDL 115
#define KEY_DEL   83
#define KEY_HOME  71
#define KEY_END   79
#define KEY_INS   82
#define KEY_DC    83
#endif
#endif

