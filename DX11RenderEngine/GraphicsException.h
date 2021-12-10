#pragma once
#include "pch.h"
#include "dxerr.h"
#include "DxgiInfoManager.h"
#include <exception>
#include <string>



// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) HrException(__LINE__,  __FILE__,  (hr))
#define GFX_THROW_NOINFO(hrcall) if(FAILED(hr = (hrcall))) throw HrException(__LINE__,  __FILE__,  hr)

#ifdef _DEBUG
#define GFX_EXCEPT(hr) HrException(__LINE__,  __FILE__, (hr), gfx.infoManager.GetMessages())
#define GFX_THROW_INFO(hrcall) gfx.infoManager.Set(); if(FAILED(hr = (hrcall))) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) DeviceRemovedException(__LINE__, __FILE__, (hr),infoManager.GetMessages())
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw InfoException(__LINE__, __FILE__, v);}}
#else
#define GFX_EXCEPT(hr) Graphics::HrException(__LINE__, __FILE__, (hr))
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException(__LINE__, __FILE__, (hr))
#define GFX_THROW_INFO_ONLY(call) (call)
#endif

#define CHWND_EXCEPT(hr) HrException(__LINE__,  __FILE__,  (hr))
#define CHWND_LAST_EXCEPT() HrException(__LINE__,  __FILE__,  GetLastError())
#define CHWND_NOGFX_EXCEPT() DeviceRemovedException(__LINE__,  __FILE__)

class GraphicsException : public std::exception {
public:
	GraphicsException(int line, const char* file) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;
private:
	int line;
	std::string file;
protected:
	mutable std::string whatBuffer;
};

class Exception : public GraphicsException {
	using GraphicsException::GraphicsException;
};
class HrException : public Exception {
public:
	HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
	const char* what() const noexcept override;
	const char* GetType() const noexcept override;
	HRESULT GetErrorCode() const noexcept;
	std::string GetErrorString() const noexcept;
	std::string GetErrorDescription() const noexcept;
	std::string GetErrorInfo() const noexcept;
private:
	HRESULT hr;
	std::string info;
};

class InfoException : public Exception {
public:
	InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept;
	const char* what() const noexcept override;
	const char* GetType() const noexcept override;
	std::string GetErrorInfo() const noexcept;
private:
	std::string info;
};

class DeviceRemovedException : public HrException {
	using HrException::HrException;
public:
	const char* GetType() const noexcept override;
private:
	std::string reason;
};