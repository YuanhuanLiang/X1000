#ifndef _TUTENG_DECODE_H
#define _TUTENG_DECODE_H

/**常用数据类型的别名定义**/
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint32;
typedef signed int int32;
typedef char CHAR;
typedef uint16 wchar;
typedef unsigned int BOOL;
typedef long long int64;
typedef unsigned long long uint64;

/**正确,错误,成功,失败宏的统一定义**/
#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE 	0
#endif

#ifndef SUCC
#define SUCC	0
#endif

#ifndef FAIL
#define FAIL	-1
#endif

/**版本控制**/
#define VERSION_RELEASE		0		/**正式版**/
#define VERSION_DEBUG		1		/**调试版**/

/**软件版本类型**/
#define VERSION_TYPE 		VERSION_RELEASE

/**版本号，范围0~99**/
#define VERSION_MASTER_ID 	0
#define VERSION_SLAVE_ID 	0
#define VERSION_REVISON_ID 	2

/**计算某个变量在结构体中相对偏移**/
#define BF_OFFSETOF(type,field) ((uint32)&((type *)0)->field)

#ifndef BF_MAX
#define BF_MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef BF_MIN
#define BF_MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef BF_ARRAY_SIZE
#define BF_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

/**实现S和D交互**/
#define BF_SWOP(S, D, T)	do{(T) = (S); (S) = (D); (D) = (T);}while(0)

/********************************************************************************
结构、枚举、公用体等结构定义
********************************************************************************/
enum {
    NO_DISPLAY,
    EN_DISPLAY,
};

/********************************************************************************
源文件函数声明

extern 外部函数声明
********************************************************************************/

typedef void (*cim_decode_get_data_cb)(uint8 *pBuf, uint32 uiBufLen);/**解码数据回调函数原型**/

int32 cim_decode_init(uint8 isEnDisplay);/**isEnDisplay, 0:不显示, 1:显示**/
int32 cim_decode_deinit(void);
char *cim_decode_get_version(void);
int32 cim_decode_get_data_register(cim_decode_get_data_cb fGetDataCb);/**注册解码回调函数**/
int32 cim_decode_start(void);/**开始解码**/
int32 cim_decode_stop(void); /**结束解码**/
int32 cim_decode_set_display(uint8 ucEnable); /**设置显示ucEnable,0:不显示,1:显示**/

#endif
