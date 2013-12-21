// 本文件是NPAPI插件开发框架文件之一，本文件可自由免费使用，修改。
// 如您对本文件有任何意见或改进建议，请联系我。如您对本文件及本文件相关的文件进行了修改请发送一份给我。
// by: JumuFENG
// email: zhcbfly@qq.com
// blog:  http://blog.csdn.net/z6482

#ifndef frmwkbase_h_
#define frmwkbase_h_

#include "npfrmwk.h"
#include <tchar.h>

struct nsPluginCreateData
{
	NPP instance;
	NPMIMEType type; 
	uint16_t mode; 
	int16_t argc;
	char** argn; 
	char** argv; 
	NPSavedData* saved;
};

class nsPluginInstanceBase
{
public:
	static NPNetscapeFuncs* NPNFuncs;

#ifdef XP_UNIX
	static char* GetPluginVersion();
	static NP_EXPORT(NPError) NS_GetValue(NPPVariable aVariable, void *aValue);
#endif

#if defined(XP_UNIX)
	static NP_EXPORT(const char*) GetMIMEDescription();
#elif defined(XP_WIN) || defined(XP_OS2)
	static const char* GetMIMEDescription();
#endif
	static nsPluginInstanceBase * NewPluginInstance(nsPluginCreateData * aCreateDataStruct);
	static void DestroyPluginInstance(nsPluginInstanceBase * aPlugin);
	// global plugin initialization and shutdown
	static NPError PluginInitialize();
	static void PluginShutdown();

	// these four methods must be implemented in the derived
	// class platform specific way
	virtual NPBool init(NPWindow* aWindow) = 0;
	virtual void shut() = 0;
	virtual NPBool isInitialized() = 0;
#ifdef ENABLE_SCRIPT_OBJECT
	//for scriptable plugins
	virtual NPObject* GetScriptableObject(){return NULL;}
#endif //ENABLE_SCRIPT_OBJECT
	// implement all or part of those methods in the derived 
	// class as needed
	virtual NPError SetWindow(NPWindow* pNPWindow)                    { return NPERR_NO_ERROR; }
	virtual NPError NewStream(NPMIMEType type, NPStream* stream, 
		NPBool seekable, uint16_t* stype)       { return NPERR_NO_ERROR; }
	virtual NPError DestroyStream(NPStream *stream, NPError reason)   { return NPERR_NO_ERROR; }
	virtual void    StreamAsFile(NPStream* stream, const char* fname) { return; }
	virtual int32_t WriteReady(NPStream *stream)                      { return 0x0fffffff; }
	virtual int32_t Write(NPStream *stream, int32_t offset, 
		int32_t len, void *buffer)                  { return len; }
	virtual void    Print(NPPrint* printInfo)                         { return; }
	virtual uint16_t HandleEvent(void* event)                         { return 0; }
	virtual void    URLNotify(const char* url, NPReason reason, 
		void* notifyData)                       { return; }
	virtual NPError GetValue(NPPVariable variable, void *value)       { return NPERR_NO_ERROR; }
	virtual NPError SetValue(NPNVariable variable, void *value)       { return NPERR_NO_ERROR; }
};


#ifdef ENABLE_SCRIPT_OBJECT
class nsScriptObjectBase : public NPObject
{
protected:
	NPP m_npp;
public:
	static NPClass nsScriptObjectClass; 
	nsScriptObjectBase(NPP npp):m_npp(npp){}
	virtual ~nsScriptObjectBase(){}

	// Virtual NPObject hooks called through this base class. Override
	// as you see fit.
	virtual void Invalidate(){}
	virtual bool HasMethod(NPIdentifier name){return false;}
	virtual bool Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result){return false;}
	virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result){return false;}
	virtual bool HasProperty(NPIdentifier name){return false;}
	virtual bool GetProperty(NPIdentifier name, NPVariant *result){return false;}
	virtual bool SetProperty(NPIdentifier name, const NPVariant *value){return false;}
	virtual bool RemoveProperty(NPIdentifier name){return false;}
	virtual bool Enumerate(NPIdentifier **identifier, uint32_t *count){return false;}
	virtual bool Construct(const NPVariant *args, uint32_t argCount, NPVariant *result){return false;}

public:
	// Static methods referenced in the NPClass
	static NPObject *_Allocate(NPP npp, NPClass *aClass);
	static void _Deallocate(NPObject *npobj);
	static void _Invalidate(NPObject *npobj);
	static bool _HasMethod(NPObject *npobj, NPIdentifier name);
	static bool _Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
	static bool _InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result);
	static bool _HasProperty(NPObject *npobj, NPIdentifier name);
	static bool _GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result);
	static bool _SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value);
	static bool _RemoveProperty(NPObject *npobj, NPIdentifier name);
	static bool _Enumerate(NPObject *npobj, NPIdentifier **value, uint32_t *count);
	static bool _Construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result);
	static NPObject *AllocateScriptPluginObject(NPP npp, NPClass *aClass);
};
#endif //ENABLE_SCRIPT_OBJECT

#endif //frmwkbase_h_