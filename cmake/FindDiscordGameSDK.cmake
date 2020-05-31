set_extra_dirs_lib(DISCORDGAMESDK discordgamesdk)
find_library(DISCORDGAMESDK_LIBRARY
  NAMES discordgamesdk
  HINTS ${HINTS_DISCORDGAMESDK_LIBDIR}
  PATHS ${PATHS_DISCORDGAMESDK_LIBDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)

set(DISCORDGAMESDK_SRC_DIR src/engine/external/discordgamesdk)
set_src(DISCORDGAMESDK_SRC GLOB ${DISCORDGAMESDK_SRC_DIR}
  achievement_manager.cpp
  achievement_manager.h
  activity_manager.cpp
  activity_manager.h
  application_manager.cpp
  application_manager.h
  core.cpp
  core.h
  discord.h
  event.h
  ffi.h
  image_manager.cpp
  image_manager.h
  lobby_manager.cpp
  lobby_manager.h
  network_manager.cpp
  network_manager.h
  overlay_manager.cpp
  overlay_manager.h
  relationship_manager.cpp
  relationship_manager.h
  storage_manager.cpp
  storage_manager.h
  store_manager.cpp
  store_manager.h
  types.cpp
  types.h
  user_manager.cpp
  user_manager.h
  voice_manager.cpp
  voice_manager.h
)

add_library(discordgamesdk EXCLUDE_FROM_ALL OBJECT ${DISCORDGAMESDK_SRC})
set(DISCORDGAMESDK_INCLUDEDIR ${DISCORDGAMESDK_SRC_DIR})
target_include_directories(discordgamesdk PRIVATE ${DISCORDGAMESDK_INCLUDEDIR})

set(DISCORDGAMESDK_DEP $<TARGET_OBJECTS:discordgamesdk>)
set(DISCORDGAMESDK_INCLUDE_DIRS ${DISCORDGAMESDK_INCLUDEDIR})
set(DISCORDGAMESDK_LIBRARIES ${DISCORDGAMESDK_LIBRARY})

# mark_as_advanced(DISCORDGAMESDK_LIBRARY DISCORDGAMESDK_INCLUDEDIR)

is_bundled(DISCORDGAMESDK_BUNDLED "${DISCORDGAMESDK_LIBRARY}")

if(DISCORDGAMESDK_BUNDLED AND TARGET_OS STREQUAL "windows")
  set(DISCORDGAMESDK_COPY_FILES
    "${EXTRA_DISCORDGAMESDK_LIBDIR}/discord_game_sdk.dll"
  )
else()
  set(DISCORDGAMESDK_COPY_FILES)
endif()

list(APPEND TARGETS_DEP discordgamesdk)

include(FindPackageHandleStandardArgs)