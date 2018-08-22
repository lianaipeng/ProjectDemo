/* 算法参考
 * https://www.cnblogs.com/zengxiangzhan/p/qqzengipdb-dat.html
 * 源码官网：
 * https://www.ipip.net/support/datx_code.html
 * 存储原理详细解说
 * https://www.cnblogs.com/kjcy8/p/5787723.html
 */

#ifndef __IPIP_H__
#define __IPIP_H__

int init(const char* ipdb);
int destroy();
int find(const char *ip, char *result);

#endif //__IPIP_H__
