/*************************************************************************
	> File Name: define.h
	> Author: kongxun 
	> Mail: kongxun.yb@gmail.com
	> Created Time: 2017年10月08日 星期日 12时06分38秒
 ************************************************************************/

#ifndef _DEFINE_H
#define _DEFINE_H

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

namespace momi {

enum PROTOCOLS { P_HTTP, P_HTTPS, FTP, FTPS, BT };

enum DOWNLOAD_TYPE { NEW_D, RESUME_D };

enum TRANSFER_TYPE { IS_MULTI, IS_SINGLE };

static const int SKIP_SSL_VERIFY                    =   1;

static const int MAX_THREAD_NUM                     =   64;

}
#endif
