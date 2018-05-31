# StdPatch

A small project that I made at the request of one person. This module removes some of limitations of the Source Engine models compiler. You just need to inject this DLL in studiomdl process.

The main feature that adds this module - opportunity to compile high-poly models.

Repository consists 2 variants of projects: Visual C++ and Delphi (obsolete since 1.3.0).

## List of compiler improvements

* Expansion of the array of vertices.
* Expansion of the array of weights.
* Expansion of the array of flexcontrollers.
* Changing the behavior of the IsInt24() function, which can make the compiler crash.
* Blocking error in SanityCheckVertexBoneLODFlags() function associated with flags.
* Vectored exceptions for more detailed description of the compilation error (since 2.0.0).
* Debug events for some compiler functions (since 2.0.0).