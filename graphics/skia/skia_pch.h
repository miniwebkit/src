
#define OS(WTF_FEATURE) (defined WTF_OS_##WTF_FEATURE  && WTF_OS_##WTF_FEATURE)

#if OS(WINDOWS)

#define _WIN32_WINNT 0x0501 

// If we don't define these, they get defined in windef.h. 
// We want to use std::min and std::max
#define max max
#define min min

#endif