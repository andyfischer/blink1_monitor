
solution "blink1_monitor"
  configurations { "Release" }
  language "C++"
  flags { "Symbols" }
  targetdir "build"
  objdir "build/obj"

  project "blink1_monitor"
    kind "ConsoleApp"
    includedirs {
        "mongoose",
        "hidapi/hidapi",
        "hidapi/mac",
        "hardware/firmware"
    }
    links {
        "IOKit.framework",
        "CoreFoundation.framework"
    }
    files {
        "blink1-lib.c",
        "blink1-monitor.cpp",
        "hidapi/mac/hid.c"
    }

-- gcc -arch i386 -arch x86_64 -pthread -mmacosx-version-min=10.6 -std=gnu99 -I ../hardware/firmware  -I./hidapi/hidapi -I./mongoose -g -c hidapi/mac/hid.c -o hidapi/mac/hid.o
-- gcc -arch i386 -arch x86_64 -pthread -mmacosx-version-min=10.6 -std=gnu99 -I ../hardware/firmware  -I./hidapi/hidapi -I./mongoose -g -c blink1-lib.c -o blink1-lib.o
-- gcc -arch i386 -arch x86_64 -pthread -mmacosx-version-min=10.6 -std=gnu99 -I ../hardware/firmware  -I./hidapi/hidapi -I./mongoose -g   -c -o blink1-tool.o blink1-tool.c
-- gcc -arch i386 -arch x86_64 -pthread -mmacosx-version-min=10.6 -std=gnu99 -I ../hardware/firmware  -I./hidapi/hidapi -I./mongoose -g -c blink1-tool.c -o blink1-tool.o
-- gcc -arch i386 -arch x86_64 -pthread -mmacosx-version-min=10.6 -std=gnu99 -I ../hardware/firmware  -I./hidapi/hidapi -I./mongoose -g  -g ./hidapi/mac/hid.o blink1-lib.o  -framework IOKit -framework CoreFoundation blink1-tool.o -o blink1-tool 
-- gcc -dynamiclib -o libBlink1.dylib -Wl,-search_paths_first -framework IOKit -framework CoreFoundation -arch i386 -arch x86_64 -pthread -mmacosx-version-min=10.6 -std=gnu99 -I ../hardware/firmware  -I./hidapi/hidapi -I./mongoose -g ./hidapi/mac/hid.o blink1-lib.o  -framework IOKit -framework CoreF
