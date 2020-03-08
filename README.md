# StdPatch

A small project that I made at the request of one person. This module removes some of limitations of the Source Engine models compiler. You just need to inject this DLL in studiomdl process. You can use **StdInjector** for it, or any other.

The main feature that adds this module - opportunity to compile high-poly models.

**Warning!** The module was developed for the Source Filmmaker compiler. Testing on other compilers was not performed; using other compilers can lead to undefined behavior.

# StdInjector

A program designed to inject the stdpatch.dll file in the studiomdl.exe process. To use, you need to put the file in the folder with studiomdl.exe. There is also argument redirection support.

**Warning!** Some antiviruses may detect this file as a virus. This is due to the interaction of the injector with another processes, which is often marked by antiviruses as suspicious activity. Disable antivirus before using any injector.

## List of compiler improvements

* Expansion of the array of vertices.
* Expansion of the array of weights.
* Expansion of the array of flexcontrollers.
* Changing the behavior of the IsInt24() function, which can make the compiler crash.
* Blocking error in SanityCheckVertexBoneLODFlags() function associated with flags.
* Vectored exceptions for more detailed description of the compilation error.
* Debug events for some compiler functions.