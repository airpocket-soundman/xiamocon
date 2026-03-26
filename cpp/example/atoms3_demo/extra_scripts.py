"""
PlatformIO extra script for the AtomS3 demo.

Adds the XMC library sources and the AtomS3 platform implementation to the
build.  Platform-specific overrides (display.cpp, ioex.cpp, battery.cpp) live
in the platform impl directory and replace the library versions.
"""

Import("env")
import os

project_dir = env.subst("$PROJECT_DIR")

def rel(*parts):
    return os.path.normpath(os.path.join(project_dir, *parts))

platform_impl_src = rel("..", "..", "..", "platform", "impl", "atoms3_pio_arduino", "src")
platform_decl_inc = rel("..", "..", "..", "platform", "decl", "include")
platform_impl_inc = rel("..", "..", "..", "platform", "impl", "atoms3_pio_arduino", "include")
lib_src           = rel("..", "..", "..", "library", "src")
lib_inc           = rel("..", "..", "..", "library", "include")

# ---- Include paths ----------------------------------------------------------
env.Append(CPPPATH=[platform_decl_inc, lib_inc])
if os.path.isdir(platform_impl_inc):
    env.Append(CPPPATH=[platform_impl_inc])

# ---- Platform implementation (includes display, ioex, battery overrides) ----
env.BuildSources("$BUILD_DIR/platform_impl", platform_impl_src)

# ---- Library: top-level sources (exclude overridden files) ------------------
env.BuildSources(
    "$BUILD_DIR/lib",
    lib_src,
    src_filter="+<*> -<display.cpp> -<ioex.cpp> -<battery.cpp>",
)

# ---- Library: subdirectories (none of these are overridden) -----------------
for entry in os.listdir(lib_src):
    subdir_path = os.path.join(lib_src, entry)
    if os.path.isdir(subdir_path):
        env.BuildSources("$BUILD_DIR/lib_%s" % entry, subdir_path)
