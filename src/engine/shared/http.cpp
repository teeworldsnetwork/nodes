/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <algorithm>
#include <base/math.h>
#include <base/system.h>
#include <engine/external/restclient/restclient-cpp/restclient.h>
#include "http.h"

CHttpRequest::CHttpRequest()
{

}

CHttp::CHttp()
{
	m_Lock = lock_create();
	m_WorkerRunning = true;
	thread_init(WorkerThread, this);
}

CHttp::~CHttp()
{
	m_WorkerRunning = false;

    int64 t = time_get();
    while (lock_trylock(m_Lock) != 0)
    {
        if (time_get() - t > time_freq())
            break;

        thread_sleep(100);
    }

    lock_unlock(m_Lock);
    lock_destroy(m_Lock);
}

int CHttp::ExecuteRequest(CHttpRequest* pRequest)
{
    m_apWorkerRequests.push_back(pRequest);
    return m_apWorkerRequests.size() - 1;
}

CHttpRequest* CHttp::PrepareRequest(std::string Url)
{
    CHttpRequest* pRequest = new CHttpRequest();
    pRequest->m_Url = Url;
    return pRequest;
}

void CHttp::WorkerThread(void* pUser)
{
    CHttp* pSelf = (CHttp*)pUser;
    lock_wait(pSelf->m_Lock);
    while (pSelf->m_WorkerRunning)
    {
        if (!pSelf->m_apWorkerRequests.size())
        {
            thread_sleep(50);
            continue;
        }

        CHttpRequest* pRequest = pSelf->m_apWorkerRequests.front();
        pSelf->m_apWorkerRequests.erase(std::find(pSelf->m_apWorkerRequests.begin(), pSelf->m_apWorkerRequests.end(), pRequest));

        // do work here
        if (pRequest->m_Data.empty())
        {
            RestClient::Response Res = RestClient::get(pRequest->m_Url);

            //callback
            if (pRequest->m_pCallback)
                pRequest->m_pCallback(pRequest->m_pUser, Res.code, Res.body);
        }
        else
        {
            RestClient::Response Res = RestClient::post(pRequest->m_Url, "application/json", pRequest->m_Data);

            //callback
            if (pRequest->m_pCallback)
                pRequest->m_pCallback(pRequest->m_pUser, Res.code, Res.body);
        }

        delete pRequest;

        thread_yield();
    }

    lock_unlock(pSelf->m_Lock);
}

IHttp* CreateHttpWrapper() { return new CHttp; }