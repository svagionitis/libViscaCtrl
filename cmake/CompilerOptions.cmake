# Global compiler options (static analysis) for C++
include (CheckCCompilerFlag)
include (CheckCXXCompilerFlag)
set(CMAKE_CXX_FLAGS_analysis "")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_analysis "-Werror -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wformat=2")
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wswitch-default -Wswitch-enum")

    # C++ specific warnings
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wnon-virtual-dtor")
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Woverloaded-virtual")
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wctor-dtor-privacy")
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wold-style-cast")
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wsign-promo")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC specific warnings (some may not be available in Clang)
        set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wsuggest-attribute=pure")
        set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wsuggest-attribute=const")
        set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wsuggest-attribute=noreturn")
        set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wsuggest-attribute=format")
        set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wtrampolines")
        set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wnoexcept")
    endif ()

    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wconversion -Wcast-align -fno-common")
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wmissing-prototypes")
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wcast-qual")
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wmissing-declarations")
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wwrite-strings")

    # Check for Clang documentation flag
    check_cxx_compiler_flag("-Wdocumentation" _WDOCUMENATATION)
    if (_WDOCUMENATATION)
        set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wdocumentation")
    endif ()

    # Check for duplicated condition flag
    check_cxx_compiler_flag("-Wduplicated-cond" _WDUPLICATED_COND)
    if (_WDUPLICATED_COND)
        set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wduplicated-cond")
    else ()
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(COMPILER_TOO_OLD 1)
        endif ()
    endif ()

    # Check for null dereference flag
    check_cxx_compiler_flag("-Wnull-dereference" _WNULL_DEREFERENCE)
    if (_WNULL_DEREFERENCE)
        set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -Wnull-dereference")
    else ()
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(COMPILER_TOO_OLD 1)
        endif ()
    endif ()

    unset (_WDOCUMENTATION)
    unset (_WDUPLICATED_COND)
    unset (_WNULL_DEREFERENCE)

elseif (MSVC)
    # A variable to enable/disable inline functionality of stdio functions.
    set(NO_CRT_STDIO_INLINE OFF)

    # The structure timespec is defined in pthread.h. We need to define
    # the HAVE_STRUCT_TIMESPEC in order not to use that one
    set(CMAKE_CXX_FLAGS_analysis "/W4 /Wall /GS /analyze -DHAVE_STRUCT_TIMESPEC")

    # Additional MSVC C++ specific warnings
    set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} /w44265 /w44061 /w44062 /w44287 /w44296 /w44311")

    # In the latest version of MSVC 1900 the stdio functions are defined
    # as inline which causes warning C4710 that the stdio functions are
    # not inlined. To disable these warnings, we disable the inline definition
    # of those functions.
    if (MSVC_VERSION EQUAL 1900)
        set(CMAKE_CXX_FLAGS_analysis "${CMAKE_CXX_FLAGS_analysis} -D_NO_CRT_STDIO_INLINE")
        set(NO_CRT_STDIO_INLINE ON)
    endif()
endif ()

# debug flags / options
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # No optimisation (-O0 or not specified) doesn't generate any warnings when
    # using gcc, which is cmake's default, so set a minimum level.
    # Also, -g3 -ggdb3 includes more debug info than the standard -g, which
    # corresponds to -g2. This adds specific gdb extensions, and level 3 adds
    # macro definitions.
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Clang doesn't support -Og
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1")
    endif ()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb")
endif ()

# See https://www.owasp.org/index.php/C-Based_Toolchain_Hardening
#     https://fedoraproject.org/wiki/Security_Features?rd=Security/Features
#     https://wiki.debian.org/Hardening
#     https://wiki.debian.org/HardeningWalkthrough
#     https://wiki.debian.org/ReleaseGoals/SecurityHardeningBuildFlags
option(FORTIFY_SOURCES "Option to build with various fortification flags" ON)
set(CMAKE_CXX_FLAGS_fortify "")

if (FORTIFY_SOURCES)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # dpkg-buildflags --get CPPFLAGS
        check_cxx_compiler_flag("-Wdate-time" _WDATE_TIME)
        if (_WDATE_TIME)
            set(CMAKE_CXX_FLAGS_fortify "-Wdate-time")
        else ()
            set(COMPILER_TOO_OLD 1)
        endif ()
        unset (_WDATE_TIME)
        add_definitions(-D_FORTIFY_SOURCE=2)

        # dpkg-buildflags --get CFLAGS
        check_cxx_compiler_flag("-fstack-protector-strong" _FSTACK_PROTECTOR_STRONG)
        if (_FSTACK_PROTECTOR_STRONG)
            set(CMAKE_CXX_FLAGS_fortify "${CMAKE_CXX_FLAGS_fortify} -fstack-protector-strong")
        else ()
            set(CMAKE_CXX_FLAGS_fortify "${CMAKE_CXX_FLAGS_fortify} -fstack-protector --param=ssp-buffer-size=4")
        endif ()
        unset(_FSTACK_PROTECTOR_STRONG)

        # dpkg-buildflags --get LDFLAGS
        set(visca_link_flags "-Wl,-z,relro")
        # additional recommended flags
        set(visca_link_flags "${visca_link_flags} -Wl,-z,noexecstack -Wl,-z,noexecheap -Wl,-z,now")
        set(visca_link_flags "${visca_link_flags} -Wl,-z,nodlopen -Wl,-z,nodump")
        set(visca_link_flags "${visca_link_flags} -Wl,--exclude-libs,ALL")

        set(CMAKE_EXE_CXX_FLAGS_fortify "-fPIE")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

    elseif (MSVC)
        set(visca_link_flags "/dynamicbase /NXCOMPAT /SafeSEH")

        # If the NO_CRT_STDIO_INLINE is enabled, there are some linking errors,
        # unresolved external symbol. In order to resolve them, need to add the
        # legacy_stdio_definitions.lib.
        # See https://msdn.microsoft.com/en-us/library/bb531344.aspx
        if (NO_CRT_STDIO_INLINE AND MSVC_VERSION EQUAL 1900)
            set(visca_link_flags "${visca_link_flags} legacy_stdio_definitions.lib")
        endif()
    endif ()

    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${visca_link_flags}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${visca_link_flags}")
    unset(visca_link_flags)
endif ()

if (COMPILER_TOO_OLD)
    message(WARNING "Your compiler is very old, please consider upgrading!")
endif ()

# Set the final CXX flags (but don't set CMAKE_CXX_FLAGS directly here
# to allow the main CMakeLists.txt to combine them appropriately)
# Note: The main CMakeLists.txt should append these to CMAKE_CXX_FLAGS

# add CPPFLAGS from environment, https://cmake.org/Bug/view.php?id=12928
add_definitions($ENV{CPPFLAGS})
