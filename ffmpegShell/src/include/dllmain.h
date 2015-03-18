// dllmain.h : Declaration of module class.

class CffmpegShellModule : public ATL::CAtlDllModuleT< CffmpegShellModule >
{
public :
	DECLARE_LIBID(LIBID_ffmpegShellLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_FFMPEGSHELL, "{CE16AB4E-030C-4008-8845-72145FBC8A5E}")
};

extern class CffmpegShellModule _AtlModule;
