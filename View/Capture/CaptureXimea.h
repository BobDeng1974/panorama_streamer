#pragma once

#include <iostream>


#include "xiApi.h"
#include "m3Api.h"



#include "Capture.h"

/**********************************************************************************/

class CaptureXimea : public D360::Capture
{
public:
	enum CaptureMode {
		FRAMERATE = 1000,
		FREERUN = 1001,
		RISINGEDGE = 1002
	};

    CaptureXimea()
	{ 
		m_captureDomain = D360::Capture::CAPTURE_XIMEA;
		init(); 
	}
    virtual ~CaptureXimea() { close(); }

    virtual bool   open( int index );
	virtual bool   start();
    virtual void   close();
    virtual double getProperty( int );
    virtual bool   setProperty( int prop, double rate = 0.0f );
	virtual bool   grabFrame(QImage& imageFrame);
    
	void retrieveMatFrame(int, QImage& imageFrame);

	virtual bool retrieveFrame(int, QImage& imageFrame);
	void reset(QImage& imageFrame);

    
private:
    void init();
    void errMsg( const char* msg, int errNum );
   
    int  getBpp();


    HANDLE    hmv;
    DWORD     numDevices;
    int       timeout;
    XI_IMG    m_xiimage;

	CaptureMode mMode;
};
