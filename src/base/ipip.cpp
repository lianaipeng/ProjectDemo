#include "ipip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Log.h"

typedef unsigned char byte;
typedef unsigned int uint;

/* 小端(Little Endian) */
#define B2IL(b) (((b)[0] & 0xFF) | (((b)[1] << 8) & 0xFF00) | (((b)[2] << 16) & 0xFF0000) | (((b)[3] << 24) & 0xFF000000))
/* 大端(Big Endian) */
#define B2IU(b) (((b)[3] & 0xFF) | (((b)[2] << 8) & 0xFF00) | (((b)[1] << 16) & 0xFF0000) | (((b)[0] << 24) & 0xFF000000))

static struct {
    byte *data;
    byte *index;
    uint *flag;
    uint offset; // 索引的绝对偏移量
} IPInfo;

int destroy() 
{
    if (!IPInfo.offset) {
        return 0;
    }
    free(IPInfo.flag);
    free(IPInfo.index);
    free(IPInfo.data);
    IPInfo.offset = 0;
    return 0;
}

int init(const char* ipdb) 
{
    if (IPInfo.offset) {
        return 0;
    }
    FILE *file = fopen(ipdb, "rb");
    fseek(file, 0, SEEK_END);
    /* 函数 ftell 用于得到文件位置指针当前位置相对于文件首的偏移字节数 */
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    /* 文件内容 */
    IPInfo.data = (byte *)malloc(size * sizeof(byte));
    fread(IPInfo.data, sizeof(byte), (size_t)size, file);
    
    fclose(file);
    
    /* 文件头 
     * 获取索引区的绝对偏移量 */
    uint indexLength = B2IU(IPInfo.data);
    /*
    Log_Debug("###### size:%d indexLength:%d", size, indexLength);
    Log_Debug("###### datx_header:%d %d %d %d", IPInfo.data[0], IPInfo.data[1], IPInfo.data[2], IPInfo.data[3]);
    Log_Debug("###### data[3]:%d data[2]:%d data[1]:%d data[0]%d", 
            IPInfo.data[3] & 0xFF, 
            (IPInfo.data[2] << 8) & 0xFF00, 
            (IPInfo.data[1] << 16) & 0xFF0000, 
            (IPInfo.data[0] << 24) & 0xFF000000);
    */

    /* 记录区
     * 文件头只有四个字节
     * Log_Debug("###### datx_header:%d %d %d %d", IPInfo.data[4], IPInfo.data[5], IPInfo.data[6], IPInfo.data[7]);
     * 显示 0 0 0 0 。 应该是IP地址 
     * */
    IPInfo.index = (byte *)malloc(indexLength * sizeof(byte));
    memcpy(IPInfo.index, IPInfo.data + 4, indexLength);
    
    /* 索引区
     * 绝对偏移量 */
    IPInfo.offset = indexLength;
    
    /*  */
    //IPInfo.flag = (uint *)malloc(256*256 * sizeof(uint));
    IPInfo.flag = (uint *)malloc(65536 * sizeof(uint));
    memcpy(IPInfo.flag, IPInfo.index, 65536 * sizeof(uint));
    
    return 0;
}

int find(const char *ip, char *result) 
{
    uint ips[4];
    int num = sscanf(ip, "%d.%d.%d.%d", &ips[0], &ips[1], &ips[2], &ips[3]);
    //Log_Debug("###### num:%d ips[0]:%d ips[1]:%d ips[2]:%d ips[3]:%d", num, ips[0], ips[1], ips[2], ips[3]);
    if (num == 4) {
        /* 查找IP的前两位  */
        uint ip_prefix_value    = ips[0] * 256 + ips[1];
        /* 查找IP的32整型值 也即索引值 */
        uint ip2long_value      = B2IU(ips);
        /*  */
        uint start              = IPInfo.flag[ip_prefix_value];
        /*  */
        //uint max_comp_len       = IPInfo.offset - 256*1024 - 4;
        uint max_comp_len       = IPInfo.offset - 262144 - 4;
        uint index_offset       = 0;
        uint index_length       = 0;

        //for (start = start * 9 + 256*1024; start < max_comp_len; start += 9) {
        for (start = start * 9 + 262144; start < max_comp_len; start += 9) {
            if (B2IU(IPInfo.index + start) >= ip2long_value) {
                index_offset = B2IL(IPInfo.index + start + 4) & 0x00FFFFFF;
                index_length = (IPInfo.index[start+7] << 8) + IPInfo.index[start+8];
                break;
            }
        }
        memcpy(result, IPInfo.data + IPInfo.offset + index_offset - 262144, index_length);
        result[index_length] = '\0';
        //Log_Debug("###### %s", result);
    }
    return 0;
}
