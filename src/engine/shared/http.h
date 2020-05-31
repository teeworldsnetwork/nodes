/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SHARED_HTTP_H
#define ENGINE_SHARED_HTTP_H

#include <engine/http.h>

class CHttp : public IHttp
{
    LOCK m_Lock;
    bool m_WorkerRunning;
    std::vector<CHttpRequest*> m_apWorkerRequests;

public:

	CHttp();
    ~CHttp();

    virtual CHttpRequest* PrepareRequest(std::string Url);

    virtual int ExecuteRequest(CHttpRequest* pRequest);

    static void WorkerThread(void* pUser);
};

#endif
