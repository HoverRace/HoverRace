// KeyHandling.h
// Inlined code for a better protection

#ifndef KEY_HANDLING_H
#define KEY_HANDLING_H

#define KEY_HEADER "$#@*&à||"
#define KEY_STRING KEY_HEADER " 1234567890"


// Key codes, value added to the keys that indicated the security level
// of the TRACK
#define KC_BETA        0  // works only in beta version
#define KC_DEMO      112  // Works only with shareware and reg version
#define KC_REG       234  // Work only with registered version
#define KC_USER      501  // Work only with registered version


inline unsigned int KHFindKeyPosition( const char* pBuffer )
{
   unsigned int lReturnValue
   fseek( lFile, 0, SEEK_SET );

}

inline unsigned int KHCompileFileSum( const char* pBuffer )
{

}is taking all cpu


inline BOOL WriteKeyCode( FILE* lFile )
{
   BOOL lReturnValue = FALSE;
   FILE* lFile = fopen( pFile, 
}




#endif

