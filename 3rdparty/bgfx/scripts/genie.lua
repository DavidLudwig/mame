--
-- Copyright 2010-2015 Branimir Karadzic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--

newoption {
	trigger = "with-tools",
	description = "Enable building tools.",
}

newoption {
	trigger = "with-shared-lib",
	description = "Enable building shared library.",
}

newoption {
	trigger = "with-sdl",
	description = "Enable SDL entry.",
}

newoption {
	trigger = "with-ovr",
	description = "Enable OculusVR integration.",
}

solution "bgfx"
	configurations {
		"Debug",
		"Release",
	}

	if _ACTION == "xcode4" then
		platforms {
			"Universal",
	}
	else
		platforms {
			"x32",
			"x64",
--			"Xbox360",
			"Native", -- for targets where bitness is not specified
	}
	end

	language "C++"
	startproject "example-00-helloworld"

BGFX_DIR = path.getabsolute("..")
local BGFX_BUILD_DIR = path.join(BGFX_DIR, ".build")
local BGFX_THIRD_PARTY_DIR = path.join(BGFX_DIR, "3rdparty")
BX_DIR = path.getabsolute(path.join(BGFX_DIR, "../bx"))

defines {
	"BX_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS=1"
}

dofile (path.join(BX_DIR, "scripts/toolchain.lua"))
if not toolchain(BGFX_BUILD_DIR, BGFX_THIRD_PARTY_DIR) then
	return -- no action specified
end

function copyLib()
end

if _OPTIONS["with-sdl"] then
	if os.is("windows") then
		if not os.getenv("SDL2_DIR") then
			print("Set SDL2_DIR enviroment variable.")
		end
	end
end

function exampleProject(_name)

	project ("example-" .. _name)
		uuid (os.uuid("example-" .. _name))
		kind "WindowedApp"

	configuration {}

	-- don't output debugdir for winphone builds
	if "winphone81" ~= _OPTIONS["vs"] then
		debugdir (path.join(BGFX_DIR, "examples/runtime"))
	end

	includedirs {
		path.join(BX_DIR,   "include"),
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty"),
		path.join(BGFX_DIR, "examples/common"),
	}

	files {
		path.join(BGFX_DIR, "examples", _name, "**.c"),
		path.join(BGFX_DIR, "examples", _name, "**.cpp"),
		path.join(BGFX_DIR, "examples", _name, "**.h"),
	}

	links {
		"bgfx",
		"example-common",
	}

	if _OPTIONS["with-sdl"] then
		defines { "ENTRY_CONFIG_USE_SDL=1" }
		links   { "SDL2" }

		configuration { "x32", "windows" }
			libdirs { "$(SDL2_DIR)/lib/x86" }

		configuration { "x64", "windows" }
			libdirs { "$(SDL2_DIR)/lib/x64" }

		configuration {}
	end

	if _OPTIONS["with-ovr"] then
		links   {
			"winmm",
			"ws2_32",
		}

		configuration { "x32" }
			libdirs { path.join("$(OVR_DIR)/LibOVR/Lib/Win32", _ACTION) }

		configuration { "x64" }
			libdirs { path.join("$(OVR_DIR)/LibOVR/Lib/x64", _ACTION) }

		configuration { "x32", "Debug" }
			links { "libovrd" }

		configuration { "x32", "Release" }
			links { "libovr" }

		configuration { "x64", "Debug" }
			links { "libovr64d" }

		configuration { "x64", "Release" }
			links { "libovr64" }

		configuration {}
	end

	configuration { "vs*" }
		linkoptions {
			"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
		}
		links { -- this is needed only for testing with GLES2/3 on Windows with VS2008
			"DelayImp",
		}

	configuration { "vs201*" }
		linkoptions { -- this is needed only for testing with GLES2/3 on Windows with VS201x
			"/DELAYLOAD:\"libEGL.dll\"",
			"/DELAYLOAD:\"libGLESv2.dll\"",
		}

	configuration { "mingw*" }
		targetextension ".exe"

	configuration { "vs20* or mingw*" }
		links {
			"gdi32",
			"psapi",
		}

	configuration { "winphone8*"}
		removelinks {
			"DelayImp",
			"gdi32",
			"psapi"
		}
		links {
			"d3d11",
			"dxgi"
		}
		linkoptions {
			"/ignore:4264" -- LNK4264: archiving object file compiled with /ZW into a static library; note that when authoring Windows Runtime types it is not recommended to link with a static library that contains Windows Runtime metadata
		}
		-- WinRT targets need their own output directories are build files stomp over each other
		targetdir (path.join(BGFX_BUILD_DIR, "arm_" .. _ACTION, "bin", _name))
		objdir (path.join(BGFX_BUILD_DIR, "arm_" .. _ACTION, "obj", _name))

	configuration { "mingw-clang" }
		kind "ConsoleApp"

	configuration { "android*" }
		kind "ConsoleApp"
		targetextension ".so"
		linkoptions {
			"-shared",
		}
		links {
			"EGL",
			"GLESv2",
		}

	configuration { "nacl*" }
		kind "ConsoleApp"
		targetextension ".nexe"
		links {
			"ppapi",
			"ppapi_gles2",
			"pthread",
		}

	configuration { "pnacl" }
		kind "ConsoleApp"
		targetextension ".pexe"
		links {
			"ppapi",
			"ppapi_gles2",
			"pthread",
		}

	configuration { "asmjs" }
		kind "ConsoleApp"
		targetextension ".bc"

	configuration { "linux-*" }
		links {
			"X11",
			"GL",
			"pthread",
		}

	configuration { "rpi" }
		links {
			"X11",
			"GLESv2",
			"EGL",
			"bcm_host",
			"vcos",
			"vchiq_arm",
			"pthread",
		}

	configuration { "osx" }
		files {
			path.join(BGFX_DIR, "examples/common/**.mm"),
		}
		links {
			"Cocoa.framework",
			"OpenGL.framework",
		}

	configuration { "ios*" }
		kind "ConsoleApp"
		files {
			path.join(BGFX_DIR, "examples/common/**.mm"),
		}
		linkoptions {
			"-framework CoreFoundation",
			"-framework Foundation",
			"-framework OpenGLES",
			"-framework UIKit",
			"-framework QuartzCore",
		}

	configuration { "xcode4", "ios" }
		kind "WindowedApp"
		files {
			path.join(BGFX_DIR, "examples/runtime/iOS-Info.plist"),
		}

	configuration { "qnx*" }
		targetextension ""
		links {
			"EGL",
			"GLESv2",
		}

	configuration {}

	strip()
end

dofile "bgfx.lua"

group "examples"
dofile "example-common.lua"

group "libs"
bgfxProject("", "StaticLib", {})

group "examples"
exampleProject("00-helloworld")
exampleProject("01-cubes")
exampleProject("02-metaballs")
exampleProject("03-raymarch")
exampleProject("04-mesh")
exampleProject("05-instancing")
exampleProject("06-bump")
exampleProject("07-callback")
exampleProject("08-update")
exampleProject("09-hdr")
exampleProject("10-font")
exampleProject("11-fontsdf")
exampleProject("12-lod")
exampleProject("13-stencil")
exampleProject("14-shadowvolumes")
exampleProject("15-shadowmaps-simple")
exampleProject("16-shadowmaps")
exampleProject("17-drawstress")
exampleProject("18-ibl")
exampleProject("19-oit")
exampleProject("20-nanovg")
exampleProject("21-deferred")
exampleProject("22-windows")
exampleProject("23-vectordisplay")
exampleProject("24-nbody")
exampleProject("25-c99")

if _OPTIONS["with-shared-lib"] then
	group "libs"
	bgfxProject("-shared-lib", "SharedLib", {})
end

if _OPTIONS["with-tools"] then
	group "tools"
	dofile "makedisttex.lua"
	dofile "shaderc.lua"
	dofile "texturec.lua"
	dofile "geometryc.lua"
end
