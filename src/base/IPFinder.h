#ifndef __IP_FINDER_H__
#define __IP_FINDER_H__

#include <string>
#include <stdio.h>

struct IPLocation {

    std::string     strCountry;
    std::string     strProvince;
    std::string     strCity;
    std::string     strCompany;
    std::string     strISP;
    std::string     strLat; // 纬度
    std::string     strLon; // 经度
    // ......

    static std::string FormatIsp(const IPLocation &loc) {
        if (loc.strProvince.empty()) {
            return loc.strISP;
        } else if (loc.strISP.empty()) {
            return loc.strProvince;
        } else {
            return loc.strProvince + "_" + loc.strISP;
        }
    }
};

class CIPFinder
{
public:
    CIPFinder();
    ~CIPFinder();

public:
    // not thread safe
    int Find(const std::string & strIP, IPLocation & loc);
    static int GetISPInfo(const char *ip, std::string &strIsp);
    static void GetLatLngInfo(const char *ip, float &lat, float &lng);
    static int ParseLocation(IPLocation & loc);
};

class CLatLng {
public:
    CLatLng(double longitude, double latitude);
    virtual ~CLatLng();

    double Radian(double d) {
        return d*PI / 180.0;	//角度1˚ = π / 180
    }

    int GetDistance(CLatLng &B); // return km

private:
    double m_longitude;
    double m_latitude ;

    const double PI;
    const double EARTH_RADIUS;
};

#endif /* __IP_FINDER_H__ */
