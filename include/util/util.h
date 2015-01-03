
/* Common HoverRace-specific utility macros. */

#pragma once

// Mark unused parameters so we can keep both the compiler and Doxygen happy.
template<class... T> void HR_UNUSED(T&&...) { }

// Mark functions as deprecated.
// Using compiler-specific methods until [[deprecated]] is widely supported.
#ifdef _WIN32
#	define HR_DEPRECATED __declspec(deprecated)
#else
#	define HR_DEPRECATED __attribute__((deprecated))
#endif
