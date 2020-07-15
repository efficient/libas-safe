Various and sundry dynamic linking–related gotchas
==================================================
This branch contains a number of small demo programs I wrote while tinkering around with dynamic
linking and other low-level userland runtime details in the process of writing the _libgotcha_
runtime.  Many of them were merely experiments thrown together to demonstrate behavior and edge
cases.  Some of them (e.g., `bind_not`, `copy`, `interpose`) implement isolated parts of
_libgotcha_'s functionality and may have some reuse value to those unafraid of reading scary,
undocumented code.  Others (e.g., `nested`, `nptl-skip_init`) were used to debug past bugs in
_libgotcha_.  Your best hope of recovering the context/history of these files is to run `git log` on
them and read the messages.

**Chances are the thing you'll find most interesting is the example program in the `posix_safety`
directory, which demonstrates how the _libas-safe_ runtime can automatically resolve deadlocks in
programs with incorrect signal handlers.**

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
