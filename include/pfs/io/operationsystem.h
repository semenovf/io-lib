#pragma once

/*
   The operating system, must be one of: (PFS_OS_x)
     DARWIN   - Darwin OS (synonym for PFS_OS_MAC)
     SYMBIAN  - Symbian
     MSDOS    - MS-DOS and Windows
     OS2      - OS/2
     OS2EMX   - XFree86 on OS/2 (not PM)
     WIN32    - Win32 (Windows 2000/XP/Vista/7 and Windows Server 2003/2008)
     WINCE    - WinCE (Windows CE 5.0)
     CYGWIN   - Cygwin
     SOLARIS  - Sun Solaris
     HPUX     - HP-UX
     ULTRIX   - DEC Ultrix
     LINUX    - Linux
     FREEBSD  - FreeBSD
     NETBSD   - NetBSD
     OPENBSD  - OpenBSD
     BSDI     - BSD/OS
     IRIX     - SGI Irix
     OSF      - HP Tru64 UNIX
     SCO      - SCO OpenServer 5
     UNIXWARE - UnixWare 7, Open UNIX 8
     AIX      - AIX
     HURD     - GNU Hurd
     DGUX     - DG/UX
     RELIANT  - Reliant UNIX
     DYNIX    - DYNIX/ptx
     QNX      - QNX
     LYNX     - LynxOS
     BSD4     - Any BSD 4.4 system
     UNIX     - Any UNIX BSD/SYSV system
*/

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  	define PFS_OS_DARWIN
#  	define PFS_OS_BSD4
#  	ifdef __LP64__
#   	define PFS_OS_DARWIN64
#  	else
#   	define PFS_OS_DARWIN32
#	endif
#elif defined(__SYMBIAN32__) || defined(SYMBIAN)
#	define PFS_OS_SYMBIAN
#	define PFS_NO_POSIX_SIGNALS
#	define PFS_NO_GETIFADDRS
#elif defined(__CYGWIN__)
#	define PFS_OS_CYGWIN
#elif defined(MSDOS) || defined(__MSDOS__) || defined(_MSDOS) /* || defined(__DOS__) */
#	define PFS_OS_MSDOS
#elif defined(__OS2__)
#	if defined(__EMX__)
#   	define PFS_OS_OS2EMX
#	else
#		define PFS_OS_OS2
#	endif
#elif !defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#	define PFS_OS_WIN32
#	define PFS_OS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#	if defined(WINCE) || defined(_WIN32_WCE)
#   	define PFS_OS_WINCE
#	else
#		define PFS_OS_WIN32
#	endif
#elif defined(__MWERKS__) && defined(__INTEL__)
#	define PFS_OS_WIN32
#elif defined(__sun) || defined(sun)
#	define PFS_OS_SOLARIS
#elif defined(hpux) || defined(__hpux)
#	define PFS_OS_HPUX
#elif defined(__ultrix) || defined(ultrix)
#	define PFS_OS_ULTRIX
#elif defined(sinix)
#	define PFS_OS_RELIANT
#elif defined(__native_client__)
#	define PFS_OS_NACL
#elif defined(__linux__) || defined(__linux)
#	define PFS_OS_LINUX
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#	define PFS_OS_FREEBSD
#	define PFS_OS_BSD4
#elif defined(__NetBSD__)
#	define PFS_OS_NETBSD
#	define PFS_OS_BSD4
#elif defined(__OpenBSD__)
#	define PFS_OS_OPENBSD
#	define PFS_OS_BSD4
#elif defined(__bsdi__)
#	define PFS_OS_BSDI
#	define PFS_OS_BSD4
#elif defined(__sgi)
#	define PFS_OS_IRIX
#elif defined(__osf__)
#	define PFS_OS_OSF
#elif defined(_AIX)
#	define PFS_OS_AIX
#elif defined(__Lynx__)
#	define PFS_OS_LYNX
#elif defined(__GNU__)
#	define PFS_OS_HURD
#elif defined(__DGUX__)
#	define PFS_OS_DGUX
#elif defined(__QNXNTO__)
#	define PFS_OS_QNX
#elif defined(_SEQUENT_)
#	define PFS_OS_DYNIX
#elif defined(_SCO_DS) /* SCO OpenServer 5 + GCC */
#	define PFS_OS_SCO
#elif defined(__USLC__) /* all SCO platforms + UDK or OUDK */
#	define PFS_OS_UNIXWARE
#elif defined(__svr4__) && defined(i386) /* Open UNIX 8 + GCC */
#	define PFS_OS_UNIXWARE
#elif defined(__INTEGRITY)
#	define PFS_OS_INTEGRITY
#elif defined(VXWORKS) /* there is no "real" VxWorks define - this has to be set in the mkspec! */
# 	define PFS_OS_VXWORKS
#elif defined(__MAKEDEPEND__)
#endif

#if defined(PFS_OS_WIN32) || defined(PFS_OS_WIN64) || defined(PFS_OS_WINCE)
#	define PFS_OS_WIN 1
#endif

#if defined(PFS_OS_DARWIN)
#	define PFS_OS_MAC  /* is mostly for compatibility */
#	define PFS_OS_MACX /* is only for compatibility */
#	if defined(PFS_OS_DARWIN64)
#   	define PFS_OS_MAC64
#	elif defined(PFS_OS_DARWIN32)
#   	define PFS_OS_MAC32
#	endif
#endif

#if defined(PFS_OS_MSDOS) || defined(PFS_OS_OS2) || defined(PFS_OS_WIN)
#	undef PFS_OS_UNIX
#   define PFS_OS_DOS
#elif !defined(PFS_OS_UNIX)
#	define PFS_OS_UNIX
#endif


#if (defined(__BORLANDC__) && __BORLANDC__ <= 0x410) \
    || (defined(__TURBOC__) && __TURBOC__ <= 0x410)
#   define PFS_OS_16BITS
#   define PFS_OS_BITS    16
#elif (defined(__WORDSIZE) && __WORDSIZE == 64)                       \
    || (defined(UINTPTR_MAX) && UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF)    \
    || defined(_WIN64)                                                \
    || (defined(__GNUC__) && (__x86_64__ || __ppc64__))               \
    || defined(__LP64__)                                              \
    || defined(_LP64)
#   define PFS_OS_64BITS
#   define PFS_OS_BITS    64
#else
#   define PFS_OS_32BITS
#   define PFS_OS_BITS    32
#endif

#if defined(PFS_OS_UNIX)
#   define PFS_OS_POSIX 1
#   define __PFS_OS_HPREFIX__ posix
#endif
