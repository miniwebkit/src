// 本文件是NPAPI插件开发框架文件之一，本文件可自由免费使用，修改。
// 如您对本文件有任何意见或改进建议，请联系我。如您对本文件及本文件相关的文件进行了修改请发送一份给我。
// by: JumuFENG
// email: zhcbfly@qq.com
// blog:  http://blog.csdn.net/z6482

#include "npfrmwkbase.h"

//
// npapi plugin functions
//

NPNetscapeFuncs* nsPluginInstanceBase::NPNFuncs = NULL;

#ifdef XP_UNIX
NP_EXPORT(char*) NP_GetPluginVersion()
{
	return nsPluginInstanceBase::GetPluginVersion();
}
#endif

#if defined(XP_UNIX)
NP_EXPORT(const char*) NP_GetMIMEDescription()
#elif defined(XP_WIN) || defined(XP_OS2)
const char* NP_GetMIMEDescription()
#endif
{
	return nsPluginInstanceBase::GetMIMEDescription();
}

#ifdef XP_UNIX
NP_EXPORT(NPError) NP_GetValue(void* future, NPPVariable aVariable, void* aValue) {
	return nsPluginInstanceBase::GetValue(aVariable,aValue);
}
#endif

static NPError fillPluginFunctionTable(NPPluginFuncs* aNPPFuncs)
{
	if (!aNPPFuncs)
		return NPERR_INVALID_FUNCTABLE_ERROR;

	// Set up the plugin function table that Netscape will use to call us.
	aNPPFuncs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
	aNPPFuncs->newp          = NPP_New;
	aNPPFuncs->destroy       = NPP_Destroy;
	aNPPFuncs->setwindow     = NPP_SetWindow;
	aNPPFuncs->newstream     = NPP_NewStream;
	aNPPFuncs->destroystream = NPP_DestroyStream;
	aNPPFuncs->asfile        = NPP_StreamAsFile;
	aNPPFuncs->writeready    = NPP_WriteReady;
	aNPPFuncs->write         = NPP_Write;
	aNPPFuncs->print         = NPP_Print;
	aNPPFuncs->event         = NPP_HandleEvent;
	aNPPFuncs->urlnotify     = NPP_URLNotify;
	aNPPFuncs->getvalue      = NPP_GetValue;
	aNPPFuncs->setvalue      = NPP_SetValue;

	return NPERR_NO_ERROR;
}

#if defined(XP_MACOSX)
NP_EXPORT(NPError) NP_Initialize(NPNetscapeFuncs* aNPNFuncs)
#elif defined(XP_WIN) || defined(XP_OS2)
NPError OSCALL NP_Initialize(NPNetscapeFuncs* aNPNFuncs)
#elif defined(XP_UNIX)
NP_EXPORT(NPError) NP_Initialize(NPNetscapeFuncs* aNPNFuncs, NPPluginFuncs* pFuncs)
#endif
{
	nsPluginInstanceBase::NPNFuncs = aNPNFuncs;

#if defined(XP_UNIX) && !defined(XP_MACOSX)
	if (!fillPluginFunctionTable(pFuncs)) {
		return NPERR_INVALID_FUNCTABLE_ERROR;
	}
#endif
	return nsPluginInstanceBase::PluginInitialize();
}

#if defined(XP_MACOSX)
NP_EXPORT(NPError) NP_GetEntryPoints(NPPluginFuncs* pFuncs)
#elif defined(XP_WIN) || defined(XP_OS2)
NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pFuncs)
#endif
#if defined(XP_MACOSX) || defined(XP_WIN) || defined(XP_OS2)
{
	return fillPluginFunctionTable(pFuncs);
}
#endif

#if defined(XP_UNIX)
NP_EXPORT(NPError) NP_Shutdown()
#elif defined(XP_WIN) || defined(XP_OS2)
NPError OSCALL NP_Shutdown()
#endif
{
	nsPluginInstanceBase::PluginShutdown();
	return NPERR_NO_ERROR;
}

//npp_functions

#ifdef XP_UNIX
char* NPP_GetMIMEDescription(void)
{
	return NS_GetMIMEDescription();
}
#endif

NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData* saved)
{
	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	// create a new plugin instance object
	// initialization will be done when the associated window is ready
	nsPluginCreateData ds;

	ds.instance = instance;
	ds.type     = pluginType; 
	ds.mode     = mode; 
	ds.argc     = argc; 
	ds.argn     = argn; 
	ds.argv     = argv; 
	ds.saved    = saved;

	nsPluginInstanceBase * plugin = nsPluginInstanceBase::NewPluginInstance(&ds);
	if (!plugin)
		return NPERR_OUT_OF_MEMORY_ERROR;

	// associate the plugin instance object with NPP instance
	instance->pdata = (void *)plugin;

	return NPERR_NO_ERROR;
}

NPError NPP_Destroy (NPP instance, NPSavedData** save)
{
	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;
	if (plugin) {
		plugin->shut();
		nsPluginInstanceBase::DestroyPluginInstance(plugin);
	}
	return NPERR_NO_ERROR;
}

NPError NPP_SetWindow (NPP instance, NPWindow* pNPWindow)
{
	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	if (!pNPWindow)
		return NPERR_GENERIC_ERROR;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;

	if (!plugin) 
		return NPERR_GENERIC_ERROR;

	// window just created
	if (!plugin->isInitialized() && pNPWindow->window) { 
		if (!plugin->init(pNPWindow)) {
			nsPluginInstanceBase::DestroyPluginInstance(plugin);
			return NPERR_MODULE_LOAD_FAILED_ERROR;
		}
	}

	// window goes away
	if (!pNPWindow->window && plugin->isInitialized())
		return plugin->SetWindow(pNPWindow);

	// window resized?
	if (plugin->isInitialized() && pNPWindow->window)
		return plugin->SetWindow(pNPWindow);

	// this should not happen, nothing to do
	if (!pNPWindow->window && !plugin->isInitialized())
		return plugin->SetWindow(pNPWindow);

	return NPERR_NO_ERROR;
}

NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype)
{
	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;
	if (!plugin) 
		return NPERR_GENERIC_ERROR;

	return plugin->NewStream(type, stream, seekable, stype);
}

int32_t NPP_WriteReady (NPP instance, NPStream *stream)
{
	if (!instance)
		return 0x0fffffff;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;
	if (!plugin) 
		return 0x0fffffff;

	return plugin->WriteReady(stream);
}

int32_t NPP_Write (NPP instance, NPStream *stream, int32_t offset, int32_t len, void *buffer)
{
	if (!instance)
		return len;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;
	if (!plugin) 
		return len;

	return plugin->Write(stream, offset, len, buffer);
}

NPError NPP_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;
	if (!plugin) 
		return NPERR_GENERIC_ERROR;

	return plugin->DestroyStream(stream, reason);
}

void NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
	if (!instance)
		return;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;
	if (!plugin) 
		return;

	plugin->StreamAsFile(stream, fname);
}

void NPP_Print (NPP instance, NPPrint* printInfo)
{
	if (!instance)
		return;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;
	if (!plugin) 
		return;

	plugin->Print(printInfo);
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
	if (!instance)
		return;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;
	if (!plugin) 
		return;

	plugin->URLNotify(url, reason, notifyData);
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;
	if (!plugin) 
		return NPERR_GENERIC_ERROR;

	return plugin->SetValue(variable, value);
}

int16_t	NPP_HandleEvent(NPP instance, void* event)
{
	if (!instance)
		return 0;

	nsPluginInstanceBase * plugin = (nsPluginInstanceBase *)instance->pdata;
	if (!plugin) 
		return 0;

	return plugin->HandleEvent(event);
}

NPError	NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
	if(instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	NPError rv = NPERR_NO_ERROR;

	nsPluginInstanceBase* plugin = (nsPluginInstanceBase*)instance->pdata;
	if(plugin == NULL)
		return NPERR_GENERIC_ERROR;
#ifdef ENABLE_SCRIPT_OBJECT
	if (variable==NPPVpluginScriptableNPObject)
	{
		*(NPObject **)value = plugin->GetScriptableObject();
	}
#endif //ENABLE_SCRIPT_OBJECT
	else rv=plugin->GetValue(variable,value);
	return rv;
}

//npn_functions

bool NPN_SetProperty(NPP instance, NPObject* obj, NPIdentifier propertyName, const NPVariant* value)
{
	return nsPluginInstanceBase::NPNFuncs->setproperty(instance, obj, propertyName, value);
}

NPIdentifier NPN_GetIntIdentifier(int32_t intid)
{
	return nsPluginInstanceBase::NPNFuncs->getintidentifier(intid);
}

NPIdentifier NPN_GetStringIdentifier(const NPUTF8* name)
{
	return nsPluginInstanceBase::NPNFuncs->getstringidentifier(name);
}

void NPN_GetStringIdentifiers(const NPUTF8 **names, int32_t nameCount, NPIdentifier *identifiers)
{
	return nsPluginInstanceBase::NPNFuncs->getstringidentifiers(names, nameCount, identifiers);
}

bool NPN_IdentifierIsString(NPIdentifier identifier)
{
	return nsPluginInstanceBase::NPNFuncs->identifierisstring(identifier);
}

NPUTF8* NPN_UTF8FromIdentifier(NPIdentifier identifier)
{
	return nsPluginInstanceBase::NPNFuncs->utf8fromidentifier(identifier);
}

int32_t NPN_IntFromIdentifier(NPIdentifier identifier)
{
	return nsPluginInstanceBase::NPNFuncs->intfromidentifier(identifier);
}

NPError NPN_GetValue(NPP instance, NPNVariable variable, void* value)
{
	return nsPluginInstanceBase::NPNFuncs->getvalue(instance, variable, value);
}

NPError NPN_SetValue(NPP instance, NPPVariable variable, void* value)
{
	return nsPluginInstanceBase::NPNFuncs->setvalue(instance, variable, value);
}

void NPN_InvalidateRect(NPP instance, NPRect* rect)
{
	nsPluginInstanceBase::NPNFuncs->invalidaterect(instance, rect);
}

bool NPN_HasProperty(NPP instance, NPObject* obj, NPIdentifier propertyName)
{
	return nsPluginInstanceBase::NPNFuncs->hasproperty(instance, obj, propertyName);
}

NPObject* NPN_CreateObject(NPP instance, NPClass* aClass)
{
	return nsPluginInstanceBase::NPNFuncs->createobject(instance, aClass);
}

bool NPN_Invoke(NPP npp, NPObject* obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return nsPluginInstanceBase::NPNFuncs->invoke(npp, obj, methodName, args, argCount, result);
}

bool NPN_InvokeDefault(NPP npp, NPObject* obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return nsPluginInstanceBase::NPNFuncs->invokeDefault(npp, obj, args, argCount, result);
}

bool NPN_Construct(NPP npp, NPObject* npobj, const NPVariant* args, uint32_t argCount, NPVariant* result)
{
	return nsPluginInstanceBase::NPNFuncs->construct(npp, npobj, args, argCount, result);
}

const char* NPN_UserAgent(NPP instance)
{
	return nsPluginInstanceBase::NPNFuncs->uagent(instance);
}

NPObject* NPN_RetainObject(NPObject* obj)
{
	return nsPluginInstanceBase::NPNFuncs->retainobject(obj);
}

void NPN_ReleaseObject(NPObject* obj)
{
	return nsPluginInstanceBase::NPNFuncs->releaseobject(obj);
}

void* NPN_MemAlloc(uint32_t size)
{
	return nsPluginInstanceBase::NPNFuncs->memalloc(size);
}

char* NPN_StrDup(const char* str)
{
	return strcpy((char*)nsPluginInstanceBase::NPNFuncs->memalloc(strlen(str) + 1), str);
}

void NPN_MemFree(void* ptr)
{
	return nsPluginInstanceBase::NPNFuncs->memfree(ptr);
}

uint32_t NPN_ScheduleTimer(NPP instance, uint32_t interval, NPBool repeat, void (*timerFunc)(NPP npp, uint32_t timerID))
{
	return nsPluginInstanceBase::NPNFuncs->scheduletimer(instance, interval, repeat, timerFunc);
}

void NPN_UnscheduleTimer(NPP instance, uint32_t timerID)
{
	return nsPluginInstanceBase::NPNFuncs->unscheduletimer(instance, timerID);
}

void NPN_ReleaseVariantValue(NPVariant *variant)
{
	return nsPluginInstanceBase::NPNFuncs->releasevariantvalue(variant);
}

NPError NPN_GetURLNotify(NPP instance, const char* url, const char* target, void* notifyData)
{
	return nsPluginInstanceBase::NPNFuncs->geturlnotify(instance, url, target, notifyData);
}

NPError NPN_GetURL(NPP instance, const char* url, const char* target)
{
	return nsPluginInstanceBase::NPNFuncs->geturl(instance, url, target);
}

NPError NPN_RequestRead(NPStream* stream, NPByteRange* rangeList)
{
	return nsPluginInstanceBase::NPNFuncs->requestread(stream, rangeList);
}

NPError NPN_PostURLNotify(NPP instance, const char* url, const char* target, uint32_t len, const char* buf, NPBool file, void* notifyData)
{
	return nsPluginInstanceBase::NPNFuncs->posturlnotify(instance, url, target, len, buf, file, notifyData);
}

NPError NPN_PostURL(NPP instance, const char *url, const char *target, uint32_t len, const char *buf, NPBool file)
{
	return nsPluginInstanceBase::NPNFuncs->posturl(instance, url, target, len, buf, file);
}

NPError NPN_DestroyStream(NPP instance, NPStream* stream, NPError reason)
{
	return nsPluginInstanceBase::NPNFuncs->destroystream(instance, stream, reason);
}

NPError NPN_NewStream(NPP instance, NPMIMEType  type, const char* target, NPStream**  stream)
{
	return nsPluginInstanceBase::NPNFuncs->newstream(instance, type, target, stream);
}

int32_t 	NPN_Write(NPP instance, NPStream* stream, int32_t len, void* buf)
{
	return nsPluginInstanceBase::NPNFuncs->write(instance, stream, len, buf);
}

bool NPN_Enumerate(NPP instance, NPObject *npobj, NPIdentifier **identifiers, uint32_t *identifierCount)
{
	return nsPluginInstanceBase::NPNFuncs->enumerate(instance, npobj, identifiers, identifierCount);
}

bool NPN_GetProperty(NPP instance, NPObject *npobj, NPIdentifier propertyName, NPVariant *result)
{
	return nsPluginInstanceBase::NPNFuncs->getproperty(instance, npobj, propertyName, result);
}

bool NPN_Evaluate(NPP instance, NPObject *npobj, NPString *script, NPVariant *result)
{
	return nsPluginInstanceBase::NPNFuncs->evaluate(instance, npobj, script, result);
}

void NPN_SetException(NPObject *npobj, const NPUTF8 *message)
{
	return nsPluginInstanceBase::NPNFuncs->setexception(npobj, message);
}

NPBool NPN_ConvertPoint(NPP instance, double sourceX, double sourceY, NPCoordinateSpace sourceSpace, double *destX, double *destY, NPCoordinateSpace destSpace)
{
	return nsPluginInstanceBase::NPNFuncs->convertpoint(instance, sourceX, sourceY, sourceSpace, destX, destY, destSpace);
}

NPError NPN_SetValueForURL(NPP instance, NPNURLVariable variable, const char *url, const char *value, uint32_t len)
{
	return nsPluginInstanceBase::NPNFuncs->setvalueforurl(instance, variable, url, value, len);
}

NPError NPN_GetValueForURL(NPP instance, NPNURLVariable variable, const char *url, char **value, uint32_t *len)
{
	return nsPluginInstanceBase::NPNFuncs->getvalueforurl(instance, variable, url, value, len);
}

NPError NPN_GetAuthenticationInfo(NPP instance,
	const char *protocol,
	const char *host, int32_t port,
	const char *scheme,
	const char *realm,
	char **username, uint32_t *ulen,
	char **password,
	uint32_t *plen)
{
	return nsPluginInstanceBase::NPNFuncs->getauthenticationinfo(instance, protocol, host, port, scheme, realm, username, ulen, password, plen);
}

void NPN_PluginThreadAsyncCall(NPP plugin, void (*func)(void*), void* userdata)
{
	return nsPluginInstanceBase::NPNFuncs->pluginthreadasynccall(plugin, func, userdata);
}

void NPN_URLRedirectResponse(NPP instance, void* notifyData, NPBool allow)
{
	return nsPluginInstanceBase::NPNFuncs->urlredirectresponse(instance, notifyData, allow);
}

#ifdef ENABLE_SCRIPT_OBJECT

/****************************************************************************\
  These are the static functions given to the browser in the NPClass struct.
  You might look at these as the "entry points" for the nsScriptObjectBase
\****************************************************************************/
NPObject* nsScriptObjectBase::_Allocate(NPP npp, NPClass *aClass)
{
	return nsScriptObjectBase::AllocateScriptPluginObject(npp,aClass);
}

void nsScriptObjectBase::_Deallocate(NPObject *npobj)
{
	//delete static_cast<nsScriptObjectBase *>(npobj);
	delete (nsScriptObjectBase*)npobj;
}

void nsScriptObjectBase::_Invalidate(NPObject *npobj)
{
	return ((nsScriptObjectBase*)npobj)->Invalidate();
}

bool nsScriptObjectBase::_HasMethod(NPObject *npobj, NPIdentifier name)
{
	return ((nsScriptObjectBase*)npobj)->HasMethod(name);
}

bool nsScriptObjectBase::_Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return ((nsScriptObjectBase*)npobj)->Invoke(name,args,argCount,result);
}

bool nsScriptObjectBase::_InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    return ((nsScriptObjectBase*)npobj)->InvokeDefault(args,argCount,result);
}

bool nsScriptObjectBase::_HasProperty(NPObject *npobj, NPIdentifier name)
{
    return ((nsScriptObjectBase*)npobj)->HasProperty(name);
}

bool nsScriptObjectBase::_GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result)
{
    return ((nsScriptObjectBase*)npobj)->GetProperty(name,result);
}

bool nsScriptObjectBase::_SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
    return ((nsScriptObjectBase*)npobj)->SetProperty(name, value);
}

bool nsScriptObjectBase::_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
    return ((nsScriptObjectBase*)npobj)->RemoveProperty(name);
}

bool nsScriptObjectBase::_Enumerate(NPObject *npobj, NPIdentifier **value, uint32_t *count)
{
    return ((nsScriptObjectBase*)npobj)->Enumerate(value, count);
}

bool nsScriptObjectBase::_Construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    return ((nsScriptObjectBase*)npobj)->Construct(args, argCount, result);
}

// This defines the "entry points"; it's how the browser knows how to create the object
// when you call NPN_CreateObject, and how it knows how to call functions on it
NPClass nsScriptObjectBase::nsScriptObjectClass = {
	NP_CLASS_STRUCT_VERSION_CTOR,
	nsScriptObjectBase::_Allocate,
	nsScriptObjectBase::_Deallocate,
	nsScriptObjectBase::_Invalidate,
	nsScriptObjectBase::_HasMethod,
	nsScriptObjectBase::_Invoke,
	nsScriptObjectBase::_InvokeDefault,
	nsScriptObjectBase::_HasProperty,
	nsScriptObjectBase::_GetProperty,
	nsScriptObjectBase::_SetProperty,
	nsScriptObjectBase::_RemoveProperty,
	nsScriptObjectBase::_Enumerate,
	nsScriptObjectBase::_Construct
};

#endif //ENABLE_SCRIPT_OBJECT
