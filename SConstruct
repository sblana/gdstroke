#!/usr/bin/env python
import os
import sys

from methods import print_error


libname = "gdstroke"
projectdir = "demo"

localEnv = Environment(tools=["default"], PLATFORM="")

localEnv["build_profile"] = "build_profile.gdbuild"

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print_error("""godot-cpp is not available within this folder, as Git submodules haven't been initialized.
Run the following command to download godot-cpp:

    git submodule update --init --recursive""")
    sys.exit(1)

env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

if env.get("is_msvc", False):
    if (env["CXXFLAGS"].count("/std:c++17")):
        env["CXXFLAGS"].remove("/std:c++17")
    env.Append(CXXFLAGS=["/std:c++20"])
else:
    if (env["CXXFLAGS"].count("-std=c++17")):
        env["CXXFLAGS"].remove("-std=c++17")
    env.Append(CXXFLAGS=["-std=c++20"])

shaders_env = env.Clone()
shaders_build_program = shaders_env.Program("temp/shaders_build_main", "src/gpu/shaders_build_main.cpp")
Default(shaders_build_program)

def run_shaders_builder(target, source, env):
    os.system(str(shaders_build_program[0]))

run_shaders_builder_command = Command("run_shaders_builder", [], run_shaders_builder)
Depends(run_shaders_builder_command, shaders_build_program)
env.SideEffect("src/gen/shaders.cpp", run_shaders_builder_command)

env.Append(CPPPATH=["src/"])

sources = Glob("src/*.cpp")
sources.append("src/gen/shaders.cpp")

if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

# .dev doesn't inhibit compatibility, so we don't need to key it.
# .universal just means "compatible with all relevant arches" so we don't need to key it.
suffix = env['suffix'].replace(".dev", "").replace(".universal", "")

lib_filename = "{}{}{}{}".format(env.subst('$SHLIBPREFIX'), libname, suffix, env.subst('$SHLIBSUFFIX'))

library = env.SharedLibrary(
    "bin/{}/{}".format(env['platform'], lib_filename),
    source=sources,
)

copy = env.Install("{}/bin/{}/".format(projectdir, env["platform"]), library)

default_args = [library, copy]
Default(*default_args)
