
// Adapted from:
// http://www.eggheadcafe.com/software/aspnet/32623009/why-does-mfc-call-crtdum.aspx

#ifdef MFCLEAKFIX_EXPORTS
#	define IGNORE_MFC_LEAKS_API __declspec(dllexport)
#else
#	define IGNORE_MFC_LEAKS_API __declspec(dllimport)
#endif

IGNORE_MFC_LEAKS_API int use_ignore_mfc_leaks(void);
