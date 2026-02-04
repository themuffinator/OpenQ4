cmake_minimum_required(VERSION 3.10)

# set the project name
project(OpenQ4)

set(Q4GAME
	game/Actor.cpp 
	game/Actor.h 
	game/AF.cpp 
	game/AF.h 
	game/AFEntity.cpp 
	game/AFEntity.h 
	game/BrittleFracture.cpp 
	game/BrittleFracture.h 
	game/Camera.cpp 
	game/Camera.h 
	game/Effect.cpp 
	game/Effect.h 
	game/Entity.cpp 
	game/Entity.h 
	game/FreeView.cpp 
	game/FreeView.h 
	game/Game.def 
	game/Game.h 
	game/GameEdit.cpp 
	game/GameEdit.h 
	game/Game_Debug.cpp 
	game/Game_Debug.h 
	game/Game_local.cpp 
	game/Game_local.h 
	game/Game_Log.cpp 
	game/Game_Log.h 
	game/Game_network.cpp 
	game/Game_render.cpp 
	game/Healing_Station.cpp 
	game/Healing_Station.h 
	game/Icon.cpp 
	game/Icon.h 
	game/IconManager.cpp 
	game/IconManager.h 
	game/IK.cpp 
	game/IK.h 
	game/Instance.cpp 
	game/Instance.h 
	game/Item.cpp 
	game/Item.h 
	game/Light.cpp 
	game/Light.h 
	game/LipSync.cpp 
	game/LipSync.h 
	game/Misc.cpp 
	game/Misc.h 
	game/Moveable.cpp 
	game/Moveable.h 
	game/Mover.cpp 
	game/Mover.h 
	game/MultiplayerGame.cpp 
	game/MultiplayerGame.h 
	game/Playback.cpp 
	game/Player.cpp 
	game/Player.h 
	game/PlayerView.cpp 
	game/PlayerView.h 
	game/Player_Cheats.cpp 
	game/Player_States.cpp 
	game/Projectile.cpp 
	game/Projectile.h 
	game/Pvs.cpp 
	game/Pvs.h 
	game/SecurityCamera.cpp 
	game/SecurityCamera.h 
	game/Sound.cpp 
	game/Sound.h 
	game/spawner.cpp 
	game/spawner.h 
	game/SplineMover.cpp 
	game/SplineMover.h 
	game/Target.cpp 
	game/Target.h 
	game/TramGate.cpp 
	game/TramGate.h 
	game/Trigger.cpp 
	game/Trigger.h 
	game/Weapon.cpp 
	game/Weapon.h 
	game/WorldSpawn.cpp 
	game/WorldSpawn.h)
	
# Quake 4 Game Code Globs	
file(GLOB Q4GAME_AI_INCLUDES game/ai/*.h)
file(GLOB Q4GAME_AI_SOURCES game/ai/*.cpp)
file(GLOB Q4GAME_ANIM_INCLUDES game/anim/*.h)
file(GLOB Q4GAME_ANIM_SOURCES game/anim/*.cpp)
file(GLOB Q4GAME_BOTS_INCLUDES game/bots/*.h)
file(GLOB Q4GAME_BOTS_SOURCES game/bots/*.cpp)
file(GLOB Q4GAME_CLIENT_INCLUDES game/client/*.h)
file(GLOB Q4GAME_CLIENT_SOURCES game/client/*.cpp)
file(GLOB Q4GAME_GAMESYS_INCLUDES game/gamesys/*.h)
file(GLOB Q4GAME_GAMESYS_SOURCES game/gamesys/*.cpp)
file(GLOB Q4GAME_MP_INCLUDES game/mp/*.h)
file(GLOB Q4GAME_MP_SOURCES game/mp/*.cpp)
file(GLOB Q4GAME_MP_STATS_INCLUDES game/mp/stats/*.h)
file(GLOB Q4GAME_MP_STATS_SOURCES game/mp/stats/*.cpp)
file(GLOB Q4GAME_PHYSICS_INCLUDES game/physics/*.h)
file(GLOB Q4GAME_PHYSICS_SOURCES game/physics/*.cpp)
file(GLOB Q4GAME_SCRIPT_INCLUDES game/script/*.h)
file(GLOB Q4GAME_SCRIPT_SOURCES game/script/*.cpp)
file(GLOB Q4GAME_VEHICLE_INCLUDES game/vehicle/*.h)
file(GLOB Q4GAME_VEHICLE_SOURCES game/vehicle/*.cpp)
file(GLOB Q4GAME_WEAPON_INCLUDES game/weapon/*.h)
file(GLOB Q4GAME_WEAPON_SOURCES game/weapon/*.cpp)

# External 
file(GLOB EXTERNAL_GLEW_INCLUDES external/glew/*.h)
file(GLOB EXTERNAL_GLEW_SOURCES  external/glew/*.c)

# Quake 4 Game Coded Source Groups
source_group("game-quake4" FILES Q4GAME)
source_group("game-quake4\\ai" FILES ${Q4GAME_AI_INCLUDES})
source_group("game-quake4\\ai" FILES ${Q4GAME_AI_SOURCES})
source_group("game-quake4\\anim" FILES ${Q4GAME_ANIM_INCLUDES})
source_group("game-quake4\\anim" FILES ${Q4GAME_ANIM_SOURCES})
source_group("game-quake4\\bots" FILES ${Q4GAME_BOTS_INCLUDES})
source_group("game-quake4\\bots" FILES ${Q4GAME_BOTS_SOURCES})
source_group("game-quake4\\client" FILES ${Q4GAME_CLIENT_INCLUDES})
source_group("game-quake4\\client" FILES ${Q4GAME_CLIENT_SOURCES})
source_group("game-quake4\\gamesys" FILES ${Q4GAME_GAMESYS_INCLUDES})
source_group("game-quake4\\gamesys" FILES ${Q4GAME_GAMESYS_SOURCES})
source_group("game-quake4\\mp" FILES ${Q4GAME_MP_INCLUDES})
source_group("game-quake4\\mp" FILES ${Q4GAME_MP_SOURCES})
source_group("game-quake4\\physics" FILES ${Q4GAME_PHYSICS_INCLUDES})
source_group("game-quake4\\physics" FILES ${Q4GAME_PHYSICS_SOURCES})
source_group("game-quake4\\script" FILES ${Q4GAME_SCRIPT_INCLUDES})
source_group("game-quake4\\script" FILES ${Q4GAME_SCRIPT_SOURCES})
source_group("game-quake4\\vehicle" FILES ${Q4GAME_VEHICLE_INCLUDES})
source_group("game-quake4\\vehicle" FILES ${Q4GAME_VEHICLE_SOURCES})
source_group("game-quake4\\weapon" FILES ${Q4GAME_WEAPON_INCLUDES})
source_group("game-quake4\\weapon" FILES ${Q4GAME_WEAPON_SOURCES})

# Engine Globs
file(GLOB ENGINE_AAS_INCLUDES aas/*.h)
file(GLOB ENGINE_AAS_SOURCES  aas/*.cpp)
file(GLOB ENGINE_BSE_INCLUDES bse/*.h)
file(GLOB ENGINE_BSE_SOURCES  bse/*.cpp)
file(GLOB ENGINE_CM_INCLUDES  cm/*.h)
file(GLOB ENGINE_CM_SOURCES   cm/*.cpp)
file(GLOB ENGINE_FRAMEWORK_INCLUDES  framework/*.h)
file(GLOB ENGINE_FRAMEWORK_SOURCES   framework/*.cpp)
file(GLOB ENGINE_FRAMEWORK_ASYNC_INCLUDES  framework/async/*.h)
file(GLOB ENGINE_FRAMEWORK_ASYNC_SOURCES   framework/async/*.cpp)
file(GLOB ENGINE_IDLIB_INCLUDES  idlib/*.h)
file(GLOB ENGINE_IDLIB_SOURCES   idlib/*.cpp)
file(GLOB ENGINE_IDLIB_ALGORITHMS_INCLUDES  idlib/algorithms/*.h)
file(GLOB ENGINE_IDLIB_ALGORITHMS_SOURCES   idlib/algorithms/*.cpp)
file(GLOB ENGINE_IDLIB_BV_INCLUDES  idlib/bv/*.h)
file(GLOB ENGINE_IDLIB_BV_SOURCES   idlib/bv/*.cpp)
file(GLOB ENGINE_IDLIB_CONTAINERS_INCLUDES  idlib/containers/*.h)
file(GLOB ENGINE_IDLIB_CONTAINERS_SOURCES   idlib/containers/*.cpp)
file(GLOB ENGINE_IDLIB_GEOMETRY_INCLUDES  idlib/geometry/*.h)
file(GLOB ENGINE_IDLIB_GEOMETRY_SOURCES   idlib/geometry/*.cpp)
file(GLOB ENGINE_IDLIB_HASHING_INCLUDES  idlib/hashing/*.h)
file(GLOB ENGINE_IDLIB_HASHING_SOURCES   idlib/hashing/*.cpp)
file(GLOB ENGINE_IDLIB_MATH_INCLUDES  idlib/math/*.h)
file(GLOB ENGINE_IDLIB_MATH_SOURCES   idlib/math/*.cpp)
file(GLOB ENGINE_IDLIB_THREADS_INCLUDES  idlib/threads/*.h)
file(GLOB ENGINE_IDLIB_THREADS_SOURCES   idlib/threads/*.cpp)
file(GLOB ENGINE_RENDERER_INCLUDES  renderer/*.h)
file(GLOB ENGINE_RENDERER_SOURCES   renderer/*.cpp)
file(GLOB ENGINE_RENDERER_COLOR_INCLUDES  renderer/color/*.h)
file(GLOB ENGINE_RENDERER_COLOR_SOURCES   renderer/color/*.cpp)
file(GLOB ENGINE_RENDERER_DXT_INCLUDES  renderer/DXT/*.h)
file(GLOB ENGINE_RENDERER_DXT_SOURCES   renderer/DXT/*.cpp)
file(GLOB ENGINE_RENDERER_JPEG_INCLUDES  renderer/jpeg-6/*.h)
file(GLOB ENGINE_RENDERER_JPEG_SOURCES   renderer/jpeg-6/*.c)
file(GLOB ENGINE_RENDERER_OPENGL_INCLUDES  renderer/OpenGL/*.h)
file(GLOB ENGINE_RENDERER_OPENGL_SOURCES   renderer/OpenGL/*.cpp)
file(GLOB ENGINE_RENDERER_SOUND_INCLUDES  renderer/sound/*.h)
file(GLOB ENGINE_RENDERER_SOUND_SOURCES   renderer/sound/*.cpp)
file(GLOB ENGINE_TOOLS_DMAP_INCLUDES  tools/compilers/dmap/*.h)
file(GLOB ENGINE_TOOLS_DMAP_SOURCES   tools/compilers/dmap/*.cpp)
file(GLOB ENGINE_SOUND_INCLUDES  sound/*.h)
file(GLOB ENGINE_SOUND_SOURCES   sound/*.cpp)
file(GLOB ENGINE_SOUND_OPENAL_INCLUDES  sound/openal/*.h)
file(GLOB ENGINE_SOUND_OPENAL_SOURCES   sound/openal/*.cpp)
file(GLOB ENGINE_SYS_INCLUDES  sys/*.h)
file(GLOB ENGINE_SYS_SOURCES   sys/*.cpp)
file(GLOB ENGINE_SYS_WIN32_INCLUDES  sys/win32/*.h)
file(GLOB ENGINE_SYS_WIN32_SOURCES   sys/win32/*.cpp)
file(GLOB ENGINE_UI_INCLUDES  ui/*.h)
file(GLOB ENGINE_UI_SOURCES   ui/*.cpp)

set(QUAKE4_SOURCES
		${ENGINE_AAS_SOURCES}
		${ENGINE_BSE_SOURCES}
		${ENGINE_CM_SOURCES}
		${ENGINE_FRAMEWORK_SOURCES}
		${ENGINE_FRAMEWORK_ASYNC_SOURCES}
		${ENGINE_IDLIB_SOURCES}
		${ENGINE_IDLIB_ALGORITHMS_SOURCES}
		${ENGINE_IDLIB_BV_SOURCES}
		${ENGINE_IDLIB_CONTAINERS_SOURCES}
		${ENGINE_IDLIB_GEOMETRY_SOURCES}
		${ENGINE_IDLIB_HASHING_SOURCES}
		${ENGINE_IDLIB_MATH_SOURCES}
		${ENGINE_IDLIB_THREADS_SOURCES}
		${ENGINE_RENDERER_SOURCES}
		${ENGINE_RENDERER_COLOR_SOURCES}
		${ENGINE_RENDERER_DXT_SOURCES}
		${ENGINE_RENDERER_JPEG_SOURCES}
		${ENGINE_RENDERER_OPENGL_SOURCES}
		${ENGINE_RENDERER_SOUND_SOURCES}
		${ENGINE_TOOLS_DMAP_SOURCES}
		${ENGINE_SOUND_SOURCES}
		${ENGINE_SOUND_OPENAL_SOURCES}
		${ENGINE_SYS_SOURCES}
		${ENGINE_SYS_WIN32_SOURCES}
		${ENGINE_UI_SOURCES}

		${EXTERNAL_GLEW_SOURCES}

		${Q4GAME}
		${Q4GAME_AI_SOURCES}
		${Q4GAME_ANIM_SOURCES}
		${Q4GAME_BOTS_SOURCES}
		${Q4GAME_CLIENT_SOURCES}
		${Q4GAME_GAMESYS_SOURCES}
		${Q4GAME_MP_SOURCES}
		${Q4GAME_MP_STATS_SOURCES}
		${Q4GAME_PHYSICS_SOURCES}
		${Q4GAME_SCRIPT_SOURCES}
		${Q4GAME_VEHICLE_SOURCES}
		${Q4GAME_WEAPON_SOURCES}
	)

if(WIN32)
	list(APPEND QUAKE4_SOURCES sys/win32/rc/OpenQ4Version.rc)
endif()


add_definitions(-DUSE_OPENAL)

if(MSVC)
add_compile_options(/J)
else()
add_compile_options(-Wno-multichar)
endif()

# add the executable
add_executable(OpenQ4 ${QUAKE4_SOURCES})
target_precompile_headers(OpenQ4 PRIVATE idLib/precompiled.h)
target_link_libraries(OpenQ4 opengl32.lib Dbghelp.lib Iphlpapi.lib dinput8.lib Winmm.lib dxguid.lib ws2_32.lib ${PROJECT_SOURCE_DIR}/external/openal-soft/lib/win64/openal32.lib)
target_include_directories(OpenQ4 PRIVATE "external/openal-soft/include")
set_target_properties(OpenQ4 PROPERTIES OUTPUT_NAME "OpenQ4" LINK_FLAGS "/STACK:16777216,16777216 /SUBSYSTEM:WINDOWS /PDB:\"OpenQ4.pdb\"")
