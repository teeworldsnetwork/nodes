/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_HTTP_H
#define ENGINE_HTTP_H

#include <vector>
#include <string>
#include "kernel.h"

class CHttpRequest
{
public:
    CHttpRequest();

    std::string m_Url;
    std::string m_Data;

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
