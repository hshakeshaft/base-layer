# Base Layer

A series of single file, header only libraries made to make my life programming in
_C_/_C++_ more pleasant, and easier.


## FAQ

### What is the license?
All source is distributed under a dual licence, of which you can choose 1. The choices
are _MIT_, and _Public Domain_; see the [LICENCE](./LICENCE) file for the full
details.

### Are these libraries in the STB Style?
Yes they are. For those unfamiliar with what this means, please consult the
[original repo](https://github.com/nothings/stb), they are excellent libraries,
and an excellent resource.

To summarise however, this simply means that each library is distributed as a single,
file, header only, library, which acts both as the API declaration and implementation.

### How do I use these?
The same way you would use any other library, with 1 important difference.

Assume a library exists in this repo, `lib_foo.h`, to use this:
- include the header in all files where you require the functionality
- then in **one** and only one **source file** do the following:

```c
#define LIB_FOO_IMPLEMENTATION
#include "lib_foo.h"
```

The general form of this looks something like:

```c
#define <library name>_IMPLEMENTATION
#include "<library name>.h"
```



### May I contribute
Yes.
