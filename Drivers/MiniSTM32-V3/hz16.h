/**************************************************************
** 	火牛开发板
**	ASCII码字库及汉字自定义字库的应用
**  版本：V1.0  
**	论坛：www.openmcu.com
**	淘宝：http://shop36995246.taobao.com/   
**  技术支持群：121939788 
***************************************************************/

#ifndef __HZ16_H
#define __HZ16_H
#include "stm32f1xx.h"


/*********************************************************************************************************
汉字字模的数据结构定义
*********************************************************************************************************/
typedef struct typefnt{                                                 /* 汉字字模数据结构             */
    unsigned char Index[2];                                             /* 汉字内码索引                 */
    unsigned char Msk[32];                                              /* 点阵码数据                   */
}FNT_GB16;

extern const FNT_GB16 GBHZ_16[];                                        /* 自定义汉字字库               */
extern const unsigned char ASCII[][16];                                 /* 自定义ASCII字库              */

extern uint16_t findHzIndex(uint8_t *hz);                                   /* 在自定义汉字库在查找所要显示 */
                                                                        /* 的汉字的区位                 */

#endif
/*********************************************************************************************************
** End of File
*********************************************************************************************************/
