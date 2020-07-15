_libas-safe_
============
This 127-line proof-of-concept library enables a program's signal handlers to call library functions
not designated by POSIX as async-signal safe.  It does this via two mechanisms: library copying
(whereby the signal handler uses a separate copy of each shared library from the rest of the process
image) and deferral (whereby signal arrivals are postponed during certain library functions'
execution).  The library is so short because _libgotcha_ does most of the heavy lifting, and so long
because _libgotcha_ doesn't currently provide a seamless way for control libraries to request a
callback at process start, and thanks for all the fish.

License
-------
The entire contents and history of this repository are distributed under the following license:
```
Copyright 2020 Carnegie Mellon University

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

Building _libas-safe_
---------------------
 1. `$ ln -s ../path/to/libgotcha`
 1. `$ make -Clibgotcha libgotcha.a libgotcha.mk`
 1. `$ make`

Using _libas-safe_
------------------
Either link your program against `libas-safe.so` or request it at load time via `$LD_PRELOAD`.

To see more details, export `$LIBASSAFE_VERBOSE`.  Depending on the test program, exporting
_libgotcha_ configuration variables such as `$LIBGOTCHA_SHAREDLIBC` may exercise different
behavior, such as forcing signals to be deferred more often.
