# Throf

## Introduction
A simple concatenative programming language influenced by Forth, Factor, and PostScript. Syntactically, Throf most closely resembles Factor and in fact, many simple Factor scripts are valid in Throf.

More interesting features, not yet implemented, will steal from ideas in PostScript and Forth most likely. Throf is growing, organically and without any real sense of direction. It is a "forever project."

## Basic Requirements

### F# Throf (fsthrof directory)
F# Throf is built using [F#](http://fsharp.org) 3.1. 

If you're a Mac user, you'll need to install [Mono](http://mono-project.com) as well. The solution file (fsthrof.sln) is compatible with Mono Develop so if you install the F# language bindings in Mono Develop, you can also build it there.

If you're a Windows user, you'll be best suited to get VS 2013 (any addition that supports F# 3.1) to build F# Throf. Additionally, it's possible to use Mono Develop with F# 3.1 on Windows and successfully compile F# Throf but it can be a pain to work with.

### C++ Throf (throf directory)
CppThrof is built with Visual Studio 2012's MSVC. It takes advantage of many C++11 features, including:

* lambdas
* unordered_set
* unordered_map
* auto

Additionally, Throf will compile with GCC 4.7.2 and above as well as clang 3.3 and above.
