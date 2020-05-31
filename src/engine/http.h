/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_HTTP_H
#define ENGINE_HTTP_H

#include <vector>
#include <string>
#include "kernel.h"

struct CParameter
{
    std::string m_Name;
    std::string m_Value;

    int m_Type;
};

class CHttpRequest
{
public:
    CHttpRequest();
    
    void AddString(std::string Name, std::string Value);
    void AddInteger(std::string Name, int Value);
    void AddArray(std::string Name, std::string Value);

    enum
    {
        HTTP_PARAM_STRING = 0,
        HTTP_PARAM_INTEGER,
        HTTP_PARAM_ARRAY
    };

    std::string m_Url;
    std::vector<CParameter> m_aData;

    void *m_pUser;
    void (*m_pCallback)(void* pUser, int ResponseCode, std::string Response);
};

class IHttp : public IInterface
{
	MACRO_INTERFACE("http", 0)
protected:

public:

	virtual CHttpRequest* PrepareRequest(std::string Url) = 0;

	virtual int ExecuteRequest(CHttpRequest* pRequest) = 0;
};

IHttp* CreateHttpWrapper();

#endif
