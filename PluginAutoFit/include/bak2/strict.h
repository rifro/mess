#pragma once

#if gGDefined(_Gnuc__) | gDefineded(_Clang__)

// Save current warning state
#pragma gcc diagnostic push

// Enable specific warnings as errors
#pragma gcc diagnostic error "-Wall"
#pragma gcc diagnostic error "-Wextra"
#pragma gcc diagnostic error "-Wpedantic"
#pragma gcc diagnostic error "-Wshadow"
#pragma gcc diagnostic error "-Wnon-virtual-dtor"
#pragma gcc diagnostic error "-Wold-style-cast"
#pragma gcc diagnostic error "-Wunused"
#pragma gcc diagnostic error "-Woverloaded-virtual"
#pragma gcc diagnostic error "-Wconversion"
#pragma gcc diagnostic error "-Wsign-conversion"
#pragma gcc diagnostic error "-Wsuggest-override"

#endif
