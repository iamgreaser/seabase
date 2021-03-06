![Your watery doom.](http://i.imgur.com/fgnjdzK.png)

----

Sea Base Omega is currently a bunch of hacked together code to draw a pretty picture.

What it's meant to be is a 2D tile-based underwater atmospheric simulation with a game tacked on.
Like Space Station 13, but underwater.

Libraries required to build:
- SDL 1.2 - http://libsdl.org/
- Lua 5.1 - http://lua.org/
- zlib - http://zlib.net/

Libraries soon to be required:
- ENet - http://enet.bespin.org/

I *was* going to use Squirrel but then I found that it kinda sucked, so I'm going with Lua.

SDL 2.0 support should be coming soon.

Unlike Iceball, there will be no TCP-based protocol, just an ENet one. Having to write that TCP stuff all over again would just be a total waste of time.

This engine is not expected to be as flexible as Iceball's, as I need to ensure that the atmos sim runs quickly.

----

LICENSING:
* The C code (the stuff in the src/ directory) is available under GPLv2.
  * The PNG loader (src/png.c) is also available under the zlib licence.
* The Lua code (pkg/.../*.lua) is available under the zlib licence.
* All assets are licensed under Creative Commons 3.0 By-Attribution Share-Alike:
  * http://creativecommons.org/licenses/by-sa/3.0/
* All documentation (unless otherwise noted) is available under CC0:
  * https://creativecommons.org/publicdomain/zero/1.0/
  * This includes this text file.
  * This might not include certain licences.
* All Makefiles are also under CC0.

