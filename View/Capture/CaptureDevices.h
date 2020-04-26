#ifndef CAPTUREDEVICES_H
#define CAPTUREDEVICES_H

#include <windows.h>
#include <dshow.h>

#include <vector>
#include <string>

#pragma comment(lib, "strmiids")

using namespace std;

class CaptureDevices
{
public:
	CaptureDevices();
	~CaptureDevices();

	HRESULT Enumerate();

	HRESULT GetVideoDevices(vector<wstring> *videoDevices);
	HRESULT GetAudioDevices(vector<wstring> *audioDevices);

	std::string getLastError();

private:
	std::string lastError;

	IEnumMoniker *enumMonikerVideo, *enumMonikerAudio;
};

#endif //CAPTUREDEVICES_H