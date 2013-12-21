// 本文件是基于NPAPI插件开发框架开发插件的必要文件之一，开发插件主要修改本文件及对应的头文件，可自由免费使用，修改。
// 如您对本文件有任何意见或改进建议，请联系我。如您对本文件及本文件相关的文件进行了修改请发送一份给我。
// by: JumuFENG
// email: zhcbfly@qq.com
// blog:  http://blog.csdn.net/z6482

#include "Plugin.h"

#define PLUGIN_VERSION     "1.0.0.1"

#define MIME_TYPES_HANDLED  "application/x-scriptobj"
#define PLUGIN_NAME         "npscriptdemo.dll"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED"::"PLUGIN_NAME
#define PLUGIN_DESCRIPTION  PLUGIN_NAME " scriptable接口插件示例" 

#ifdef XP_UNIX
char* nsPluginInstanceBase::GetPluginVersion()
{
	return PLUGIN_VERSION;
}

NP_EXPORT(NPError) nsPluginInstanceBase::NS_GetValue(NPPVariable aVariable, void *aValue)
{
	switch (aVariable) {
	case NPPVpluginNameString:
		*((char**)aValue) = PLUGIN_NAME;
		break;
	case NPPVpluginDescriptionString:
		*((char**)aValue) = PLUGIN_DESCRIPTION;
		break;
	default:
		return NPERR_INVALID_PARAM;
		break;
	}
	return NPERR_NO_ERROR;
}
#endif

#if defined(XP_UNIX)
NP_EXPORT(const char*) nsPluginInstanceBase::GetMIMEDescription()
#elif defined(XP_WIN) || defined(XP_OS2)
const char* nsPluginInstanceBase::GetMIMEDescription()
#endif
{
	return MIME_TYPES_HANDLED;
}


NPError nsPluginInstanceBase::PluginInitialize()
{
	return NPERR_NO_ERROR;
}

void nsPluginInstanceBase::PluginShutdown()
{
}

nsPluginInstanceBase * nsPluginInstanceBase::NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
	if(!aCreateDataStruct)
		return NULL;

	CPlugin * plugin = new CPlugin(aCreateDataStruct->instance);
	return plugin;
}

void nsPluginInstanceBase::DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
	if(aPlugin)
		delete (CPlugin *)aPlugin;
}


CPlugin::CPlugin(NPP npp):nsPluginInstanceBase(),
	m_pNPInstance(npp),
	m_bInitialized(FALSE)
{
#ifdef ENABLE_SCRIPT_OBJECT
	m_pScriptableObject=NULL;
	m_jsObj = NULL;
	CScriptObject::bar_id = NPN_GetStringIdentifier("bar");
	CScriptObject::func_id = NPN_GetStringIdentifier("func");
	CScriptObject::foo_id = NPN_GetStringIdentifier("foo");
	CScriptObject::str_id = NPN_GetStringIdentifier("str");
	CScriptObject::i2ifunc_id = NPN_GetStringIdentifier("funci2i");
	CScriptObject::s2sfunc_id = NPN_GetStringIdentifier("funcs2s");
	CScriptObject::jsfunc_id = NPN_GetStringIdentifier("OnJsFunc");
	NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &(CScriptObject::sWindowObj));
#endif //ENABLE_SCRIPT_OBJECT
}

NPBool CPlugin::init(NPWindow* pNPWindow)
{
	m_bInitialized = TRUE;
	return TRUE;
}

void CPlugin::shut()
{
	m_bInitialized = FALSE;
}

NPBool CPlugin::isInitialized()
{
	return m_bInitialized;
}

CPlugin::~CPlugin()
{
#ifdef ENABLE_SCRIPT_OBJECT
	if (m_jsObj)
		NPN_ReleaseObject(m_jsObj);
	if (m_pScriptableObject)
		NPN_ReleaseObject(m_pScriptableObject);
#endif //ENABLE_SCRIPT_OBJECT
}


#ifdef ENABLE_SCRIPT_OBJECT
NPObject *nsScriptObjectBase::AllocateScriptPluginObject(NPP npp, NPClass *aClass)
{
	return (NPObject*)new CScriptObject(npp);
}

NPObject* CPlugin::GetScriptableObject()
{
	if (!m_pScriptableObject) {
		m_pScriptableObject = NPN_CreateObject(m_pNPInstance,	&CScriptObject::nsScriptObjectClass);
	}

	if (m_pScriptableObject) {
		NPN_RetainObject(m_pScriptableObject);
	}

	return m_pScriptableObject;
}

NPIdentifier CScriptObject::bar_id;
NPIdentifier CScriptObject::func_id;
NPIdentifier CScriptObject::foo_id;
NPIdentifier CScriptObject::str_id;
NPIdentifier CScriptObject::i2ifunc_id;
NPIdentifier CScriptObject::s2sfunc_id;
NPIdentifier CScriptObject::jsfunc_id;
NPObject * CScriptObject::sWindowObj;

bool CScriptObject::HasMethod(NPIdentifier name)
{
	return name == func_id || name == i2ifunc_id || name == s2sfunc_id;
}

bool CScriptObject::HasProperty(NPIdentifier name)
{
	return name == bar_id || name == foo_id || name == str_id
		|| name == jsfunc_id;
}

bool CScriptObject::GetProperty(NPIdentifier name, NPVariant *result)
{
	if (name == foo_id)
	{
		INT32_TO_NPVARIANT(m_vfoo,*result);
		m_vfoo++;
		return true;
	}
	else if (name == str_id)
	{
		if(m_str=="") {VOID_TO_NPVARIANT(*result); m_str ="s"; return true;}
		char* npOutString = (char*) NPN_MemAlloc(m_str.length() + 1);
		if (!npOutString)
			return false;
		strcpy(npOutString, m_str.c_str());
		STRINGZ_TO_NPVARIANT(npOutString,*result);
		m_str += "s";
		return true;
	}
	return NPN_GetProperty(m_npp,sWindowObj,name,result);
}

bool CScriptObject::SetProperty(NPIdentifier name, const NPVariant *value)
{
	if(name==foo_id){
		if(value->type == NPVariantType_Int32)
			m_vfoo = NPVARIANT_TO_INT32(*value);
		else m_vfoo = boost::lexical_cast<int>(value);
		return true;
	}
	else if(name == str_id)
	{
		if (value->type == NPVariantType_String) m_str = std::string(value->value.stringValue.UTF8Characters);
		else if(value->type == NPVariantType_Int32) m_str = boost::lexical_cast<std::string>(value->value.intValue);
		else if(value->type == NPVariantType_Double) m_str = boost::lexical_cast<std::string>(value->value.doubleValue);
		else if(value->type == NPVariantType_Bool) m_str == boost::lexical_cast<std::string>(value->value.boolValue);
		else m_str = "";
		return true;
	}
	if (name == jsfunc_id)
	{
		CPlugin * plugin = (CPlugin*) m_npp->pdata;
		if (plugin->m_jsObj == NULL)
		{
			plugin->m_jsObj = NPN_RetainObject(NPVARIANT_TO_OBJECT(*value));
		}
		return true;
	}
	return NPN_SetProperty(m_npp,sWindowObj,name,value);
}

bool CScriptObject::Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	if (name == func_id)
	{
		MessageBox(NULL,_T("func"),_T(""),0);
		CPlugin* plugin = (CPlugin*) m_npp->pdata;
		if (!(!plugin->m_jsObj))
		{
			NPVariant result;
			NPN_InvokeDefault(m_npp,plugin->m_jsObj,NULL,0,&result);
			NPN_ReleaseVariantValue(&result);
		}
		return true;
	}
	if (name == i2ifunc_id)
	{
		int t;
		if(args[0].type == NPVariantType_Int32)
			t = NPVARIANT_TO_INT32(args[0]);
		else t = boost::lexical_cast<int>(&args[0]);
		INT32_TO_NPVARIANT(-t,*result);
		return true;
	}
	if (name == s2sfunc_id)
	{
		std::string firstr = "";
		if (args[0].type == NPVariantType_String)
		{
			firstr = std::string(args[0].value.stringValue.UTF8Characters);
		}
		std::string tmpstr = "processed string:"+ firstr;// 
		char* npOutString = (char*) NPN_MemAlloc(tmpstr.length() + 1);
		if (!npOutString)
			return false;
		strcpy(npOutString, tmpstr.c_str());
		STRINGZ_TO_NPVARIANT(npOutString,*result);
		return true;
	}
	return false;
}

bool CScriptObject::InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	return true;
}
#endif //ENABLE_SCRIPT_OBJECT
