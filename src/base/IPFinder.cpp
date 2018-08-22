#include "IPFinder.h"

#include <string.h>
#include <math.h>

#include "ipip.h"
#include "base_include.h"


CIPFinder::CIPFinder()
{
    //std::string path = CFileEx::GetExeDirectory() + "/../config/ipip.datx";
    std::string path = "./config/mydata4vipday2.datx";
    init(path.c_str());
}

CIPFinder::~CIPFinder()
{
    destroy();
}

char *strtok_r_2(char *str, char const *delims, char **context) 
{
    char *p, *ret = NULL;

    if (str != NULL)
        *context = str;

    if (*context == NULL)
        return NULL;

    if ((p = strpbrk(*context, delims)) != NULL) {
        *p = 0;
        ret = *context;
        *context = ++p;
    } else if (**context) {
        ret = *context;
        *context = NULL;
    }
    return ret;
}

int CIPFinder::Find(const std::string & strIP, IPLocation & loc)
{
    if (strIP.empty()) {
        return 0;
    }

    char result[128] = {0};
    find(strIP.c_str(), result);

    char *rst = NULL;
    char *lasts;
    int n = 0;
    rst = strtok_r_2(result, "\t", &lasts);
    while (rst) {
        if (rst && n == 0)      loc.strCountry = rst;
        if (rst && n == 1)      loc.strProvince = rst;
        if (rst && n == 2)      loc.strCity = rst;
        if (rst && n == 3)      loc.strCompany = rst;
        if (rst && n == 4)      loc.strISP = rst;
        if (rst && n == 5)      loc.strLat = rst;
        if (rst && n == 6)      loc.strLon = rst;

        n++;
        rst = strtok_r_2(NULL, "\t", &lasts);
    }
    Log_Debug("strCountry:%s strProvince:%s strCity:%s strCompany:%s strISP:%s strLat:%s strLon:%s",
            loc.strCountry.c_str(), loc.strProvince.c_str(), loc.strCity.c_str(), 
            loc.strCompany.c_str(), loc.strISP.c_str(), 
            loc.strLat.c_str(), loc.strLon.c_str());
    return 0;
}

int CIPFinder::ParseLocation(IPLocation &loc) {
    if (loc.strCountry.empty()) {
        loc.strCountry = "Other";
        loc.strProvince = "Other";
        loc.strISP = "other";
        return 0;
    }

    if (loc.strCountry[0] == -28 && loc.strCountry[1] == -72 && loc.strCountry[2] == -83 &&
        loc.strCountry[3] == -27 && loc.strCountry[4] == -101 && loc.strCountry[5] == -67)  {
        // China

    } else if (loc.strCountry[0] == -27 && loc.strCountry[1] == -79 && loc.strCountry[2] == -128 &&
               loc.strCountry[3] == -27 && loc.strCountry[4] == -97 && loc.strCountry[5] == -97  &&
               loc.strCountry[6] == -25 && loc.strCountry[7] == -67 && loc.strCountry[8] == -111) {
        //LAN
        loc.strCountry = "Other";
        loc.strProvince = "Other";
        loc.strISP = "other";

    } else if (loc.strCountry[0] == -28 && loc.strCountry[1] == -65 && loc.strCountry[2] == -99 &&
               loc.strCountry[3] == -25 && loc.strCountry[4] == -107 && loc.strCountry[5] == -103  &&
               loc.strCountry[6] == -27 && loc.strCountry[7] == -100 && loc.strCountry[8] == -80) {

        // "保留地址"
        loc.strCountry = "Other";
        loc.strProvince = "Other";
        loc.strISP = "other";

    } else {
        //Overseas
        loc.strCountry = loc.strProvince = loc.strISP = "海外";
    }

    return 0;
}

int CIPFinder::GetISPInfo(const char *ip, string &strIsp)
{
    char res[128]/* = {'\0'}*/;
    char tmp[256]/* = {'\0'}*/;
    find(ip, res);
    char *lasts;
    int i = 0;
    char* rst = strtok_r_2(res, "\t", &lasts);
    while (rst && i++ <= 4) {
        if (i != 3) {

            if (i == 1){
                strcpy(tmp,rst);
            } else {
                if (rst[0] != '\0')
                    sprintf(tmp,"%s_%s",tmp,rst);
            }
        }
        rst = strtok_r_2(NULL, "\t", &lasts);
    }

    strIsp = tmp;
    return 1;
}

void CIPFinder::GetLatLngInfo(const char *ip, float &lat, float &lng)
{
    char res[512];
    char tmp[512];
    find(ip, res);

    char *lasts;
    int i = 0;
    char* rst = strtok_r_2(res, "\t", &lasts);
    while (rst && i <= 6) {
        if (i >= 5) {
            if (i == 5) {
                strcpy(tmp,rst);
                lat = (float)(atof(tmp));
            } else {
                strcpy(tmp,rst);
                lng = (float)(atof(tmp));
            }
        }
        rst = strtok_r_2(NULL, "\t", &lasts);
        ++i;
    }
}

CLatLng::CLatLng(double longitude, double latitude) : 
    m_longitude(longitude), 
    m_latitude(latitude),
    PI(3.1415927), 
    EARTH_RADIUS(6378.137)
{

}

CLatLng::~CLatLng()
{

}

int CLatLng::GetDistance(CLatLng &B)
{
    double radLatitudeA = Radian(this->m_latitude);
    double radLatitudeB = Radian(B.m_latitude);
    double radLongitudeA = Radian(this->m_longitude);
    double radLongitudeB = Radian(B.m_longitude);
    double radLatitudeDiff = radLatitudeA - radLatitudeB;
    double radLongitudeDiff = radLongitudeA - radLongitudeB;

    double dst = 2 * asin((sqrt(pow(sin(radLatitudeDiff / 2), 2) + cos(radLatitudeA) * cos(radLatitudeB) * pow(sin(radLongitudeDiff / 2), 2) )));
    dst = dst * EARTH_RADIUS;
    dst = round(dst * 10000000) / 10000;
    return (int)(dst + 500) / 1000;
}
