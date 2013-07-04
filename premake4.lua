
solution "blink1_monitor"
  configurations { "Release" }
  language "C++"
  targetdir "build"
  objdir "build/obj"

  project "blink1_monitor"
    kind "ConsoleApp"
    includedirs {
        "mongoose",
        "hidapi/hidapi",
        "hidapi/mac",
        "../hardware/firmware"
    }
    files {
        "blink1-lib.c",
        "blink1-monitor.cpp"
    }

