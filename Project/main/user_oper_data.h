#ifndef _OPER_DATA_H_
#define _OPER_DATA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 数据相关的API */
#define ENABLE_DATA_OPER_API            1

/*字符和字符串相关的API */
#define ENABLE_CHAR_STR_OPER_API        0

/*数据校验相关的API */
#define ENABLE_DATA_CHECK_API           0


/********************************************************************************************************************/
/** @addtogroup 数据相关的API
 *  @{  
 */
#if		ENABLE_DATA_OPER_API == 1

/* 数据类型大小选择 */
typedef      uint8_t       opd_uint_t;
// typedef      uint16_t       opd_uint_t;
// typedef      uint32_t       opd_uint_t;


/**
 * @brief 返回1个位的掩码，指定位设置为1，而其他位都是0。
 * @param bit: 位编号
 * @return 新的数据
 */
#define Bit_MASK(bit)						(1U << (bit))

/**
 * @brief 返回1个范围的掩码，指定范围内的位都被设置为1，而其他位都是0。
 * @param h_bit: 高位
 * @param l_bit: 低位
 * @return 新的数据
 */
#define Bits_MASK(h_bit, l_bit)				(((1U << ((h_bit) - (l_bit) + 1)) - 1) << (l_bit))

/**
 * @brief 获取data的第bit位，并返回。
 * @param data: 被读取的原数据
 * @param bit: 位编号
 * @return 读取到的bit数据
 * @note 例：uGetBit(0xcd, 6);   // 获取0xcd的第[6]bit数据，返回结果1
 */
#define	uGetBit(data, bit)					(((data) >> (bit)) & 1U)

/**
 * @brief 将data的第bit位置一或清零，并返回。
 * @param data: 被操作的原数据
 * @param bit: 位编号
 * @param bit_val: 新的位值
 * @return 新的数据
 * @note 例：uSetBit(0xcd, 6, 0);   // 将0xcd的第[6]bit清零，返回结果0x8d
 */
#define	uSetBit(data, bit, bit_val)			(((data) & ~(1U << (bit))) | ((bit_val) << (bit)))

/**
 * @brief 获取data的第h_bit~l_bit位，并返回。
 * @param data: 被读取的原数据
 * @param h_bit: 高位
 * @param l_bit: 低位
 * @return 读取到的数据
 * @note 例：uGetBits(0xcd, 4, 2);   // 获取0xcd的[4:2]bit数据，返回结果3
 */
opd_uint_t uGetBits(opd_uint_t data, uint8_t h_bit, uint8_t l_bit);

/**
 * @brief 将data的第h_bit~l_bit位置一或清零，并返回。
 * @param data: 被操作的原数据
 * @param h_bit: 高位
 * @param l_bit: 低位
 * @param bit_val: 新的位值
 * @return 新的数据
 * @note 例：uSetBits(0xcd, 4, 2);   // 将0xcd的[4:2]bit数据置一或清零，返回结果0xc1或0xdd
 */
opd_uint_t uSetBits(opd_uint_t data, uint8_t h_bit, uint8_t l_bit, opd_uint_t bit_val);

/**
 * @brief 将data的第h_bit~l_bit位设置为NewBit，并返回。
 * @param data: 被操作的原数据
 * @param h_bit: 高位
 * @param l_bit: 低位
 * @param bit_val: 新的位值
 * @return 新的数据
 * @note 例：uWriteBits(0xcd, 4, 2, 5);   // 将NewBit写到0xcd的[4:2]bit，返回结果0xd5
 */
opd_uint_t uWriteBits(opd_uint_t data, uint8_t h_bit, uint8_t l_bit, opd_uint_t bit_val);


/// @brief 将二进制格式的整数转换为对应的整数，例如输入bin_dec = 11010100（0xA80034）时，返回 212（0xD4）
/// @param bin_dec 二进制格式的整数（每个位置只能是0或1）
/// @return 对应的整数
uint8_t bin_to_dec(uint32_t bin_dec);

#endif
/** @}*/ // 整形数据相关的API


/********************************************************************************************************************/
/** @addtogroup 字符和字符串相关的API
 *  @{  
 */
#if		ENABLE_CHAR_STR_OPER_API == 1
#include <stdio.h>
#include <string.h>

/**
 * @brief 用于判断字符串str2是否是str1的子串。
 * 		  功能等同strstr()
 * @details 如果是，则该函数返回str2在str1中首次出现的地址；否则，返回NULL。 效果等同于strstr()函数
 * @param str1: 被搜索的字符串
 * @param str2: 所搜索的字符串
 * @return char*: 若str2是str1的子串，则返回str2在str1的首次出现的地址；如果str2不是str1的子串，则返回NULL。
 * @note 例：if(strstr(xUart0.Buf_P, "PPS") != NULL )
 */
char *_strstr(const char *str1, const char *str2);

/**
 * @brief 把字符串中第一个出现的数字转换成整型数返回
 * 		  功能等同atoi()
 * @param str: 需要转换的字符串
 * @return int: 转换后的数字
 * @note 例：num = _atoi((const char *) xUart0.Buf_P);
 */
int _atoi(const char *str);

/// @brief 将十六进制表示的字符串转换为对应的整数，例如输入hex_str = "0xa1" 时，返回 161（0xA1）
/// @param hex_str 十六进制格式的字符串（可识别大小写，可识别0x/0X前缀）
/// @return 对应的整数
uint32_t hex_to_dec(const char *hex_str);

#endif
/** @}*/ // 字符和字符串相关的API

/********************************************************************************************************************/
/** @addtogroup 数据校验相关的API
 *  @{  
 */
#if		ENABLE_DATA_CHECK_API == 1

/**
 * @brief 计算数据校验和
 * @param data: 待计算校验和的数据
 * @param len: 数据长度
 * @return uint8_t: 校验和
 */
uint8_t u8_CheckSum(const uint8_t *data, uint16_t len);

#endif
/** @}*/ // 数据校验相关的API

/********************************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
