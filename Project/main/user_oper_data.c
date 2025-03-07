/**
 * @file user_oper_data.c
 * @author CAIJIAMING (943251827@qq.com)
 * @brief 常用的数据和字符处理相关API
 * @version 1.1.0
 * @date 2025-01-02
 * @copyright Copyright (c) 2024
 */

#include "user_oper_data.h"


/********************************************************************************************************************/
/** @addtogroup 整形数据相关的API
 *  @{  
 */
#if		ENABLE_DATA_OPER_API == 1

/**
 * @brief 获取data的第h_bit~l_bit位，并返回。
 * @param data: 被读取的原数据
 * @param h_bit: 高位
 * @param l_bit: 低位
 * @return 读取到的数据
 * @note 例：uGetBits(0xcd, 4, 2);   // 获取0xcd的[4:2]bit数据，返回结果3
 */
opd_uint_t uGetBits(opd_uint_t data, uint8_t h_bit, uint8_t l_bit) 
{
    opd_uint_t mask = Bits_MASK(h_bit, l_bit);

    return (data & mask) >> l_bit;                     // 使用位与操作提取data中指定的位  
}

/**
 * @brief 将data的第h_bit~l_bit位置一或清零，并返回。
 * @param data: 被操作的原数据
 * @param h_bit: 高位
 * @param l_bit: 低位
 * @param bit_val: 新的位值
 * @return 新的数据
 * @note 例：uSetBits(0xcd, 4, 2);   // 将0xcd的[4:2]bit数据置一或清零，返回结果0xc1或0xdd
 */
opd_uint_t uSetBits(opd_uint_t data, uint8_t h_bit, uint8_t l_bit, opd_uint_t bit_val) 
{
    opd_uint_t mask = Bits_MASK(h_bit, l_bit);

    if(bit_val) 
        return (data | mask);                           // 将指定位置一
    else 
        return (data & (~mask));                        // 将指定位清零
}

/**
 * @brief 将data的第h_bit~l_bit位设置为NewBit，并返回。
 * @param data: 被操作的原数据
 * @param h_bit: 高位
 * @param l_bit: 低位
 * @param bit_val: 新的位值
 * @return 新的数据
 * @note 例：uWriteBits(0xcd, 4, 2, 5);   // 将NewBit写到0xcd的[4:2]bit，返回结果0xd5
 */
opd_uint_t uWriteBits(opd_uint_t data, uint8_t h_bit, uint8_t l_bit, opd_uint_t bit_val) 
{
    opd_uint_t mask = Bits_MASK(h_bit, l_bit);

    bit_val = (bit_val << l_bit) & mask;                // 移动到指定的位置，并清除无用位
    data &= ~mask;                                      // 清除原来的位
    
    return data | bit_val;                              // 将 bit_val 写入指定的位
}


/// @brief 将二进制格式的整数转换为对应的整数，例如输入bin_dec = 11010100（0xA80034）时，返回 212（0xD4）
/// @param bin_dec 二进制格式的整数（每个位置只能是0或1）
/// @return 对应的整数
uint8_t bin_to_dec(uint32_t bin_dec) 
{
    uint8_t result = 0;
    uint8_t base = 1; // 2^0

    while (bin_dec > 0) {
        uint8_t last_digit = bin_dec % 10;
        result += last_digit * base;
        base *= 2;
        bin_dec /= 10;
    }

    return result;
}


#endif
/** @}*/ // 整形数据相关的API


/********************************************************************************************************************/
/** @addtogroup 字符和字符串相关的API
 *  @{  
 */
#if		ENABLE_CHAR_STR_OPER_API == 1

/**
 * @brief 用于判断字符串str2是否是str1的子串。
 * 		  功能等同strstr()
 * @details 如果是，则该函数返回str2在str1中首次出现的地址；否则，返回NULL。 效果等同于strstr()函数
 * @param str1: 被搜索的字符串
 * @param str2: 所搜索的字符串
 * @return char*: 若str2是str1的子串，则返回str2在str1的首次出现的地址；如果str2不是str1的子串，则返回NULL。
 * @note 例：if(strstr(xUart0.Buf_P, "PPS") != NULL )
 */
char *_strstr(const char *str1, const char *str2)
{
	char *p = (char *)str1;
	char *s = (char *)str2;
	char *q = NULL;
		
	while(*p) {
		q = p;		
		while(*p && *s && (*p==*s)) {
			p++;
			s++;
		}
		if(*s == '\0')
            return q;			

		p++;
	}
	
	return NULL;	
}

/**
 * @brief 把字符串中第一个出现的数字转换成整型数返回
 * 		  功能等同atoi()
 * @param str: 需要转换的字符串
 * @return int: 转换后的数字
 * @note 例：num = _atoi((const char *) xUart0.Buf_P);
 */
int _atoi(const char *str)
{
	int result = 0;
	int sign = 1; // 1表示正数，-1表示负数
    
	// 跳过空白字符
	while(*str == ' ') {
		str++;
	}
    
	// 处理正负号
	if(*str == '-') {
		sign = -1;
		str++;
	}
	else if(*str == '+') {
		str++;
	}
    
	while(*str != '\0') {
		// 转换字符为整数
		if(*str >= '0' && *str <= '9')
		{
			result = result * 10 + (*str - '0');			
		}		
		str++;
	}

	return sign * result;
}


/// @brief 将十六进制表示的字符串转换为对应的整数，例如输入hex_str = "0xa1" 时，返回 161（0xA1）
/// @param hex_str 十六进制格式的字符串（可识别大小写，可识别0x/0X前缀）
/// @return 对应的整数
uint32_t hex_to_dec(const char *hex_str) 
{
    uint32_t result = 0;

	// printf("hex_str = %s\n", hex_str);

    // 使用 sscanf 解析十六进制字符串，%x 或 %X 用于匹配十六进制数
    if (sscanf(hex_str, "%x", &result) != 1) {
        // 处理解析错误的情况
        // printf("Invalid hex string: %s\n", hex_str);
        return 0;
    }
    return result;
}


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
uint8_t u8_CheckSum(const uint8_t *data, uint16_t len) 
{
    uint16_t sum = 0;
	uint16_t i;

    for (i = 0; i < len; i++) {
        sum += data[i];
    }

    return (uint8_t)(sum & 0xFF); // 返回低 8 位作为校验和
}


#endif
/** @}*/ // 数据校验相关的API

/********************************************************************************************************************/


