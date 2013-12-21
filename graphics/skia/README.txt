===== What's this? =====
This is a clone of Google Skia (website here) on 2011/6/27. The original project needs python to generate the project file to build it. I modify it a bit, removed many unrelated things (examples). And added VS2010 project file. You can then build Skia on Windows with a few clicks in VS2010.

===== Something worth mention =====
1. Removed SkFlate.cpp and its belonging project "zlib"
2. Moved SkGlyphCache.h from src/core to include/core