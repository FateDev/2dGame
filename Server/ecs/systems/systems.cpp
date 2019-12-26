#include "../../header/ecs.h"
#include "../../header/helper.h"

#include <cmath>
#include <functional>
#include <SFML/Network.hpp>
#include <curl/curl.h>
#include <chrono>

using namespace ecs::system;
using namespace ecs::component;

/*
༶•┈┈⛧┈♛ ♛┈⛧┈┈•༶
\\ 。°。°。°。°。°。°。°。°。°。°。°
\\\  .・。.・゜✭・.・✫・゜・。.
\\\\  .｡*ﾟ+.*.｡　　　ﾟ+..｡*ﾟ+
\\\\\ ｡･:*:･ﾟ★,｡･:*:･ﾟ☆　　 ｡･:*:･ﾟ★,｡･:*:･ﾟ☆
||||| This is the place to prototype code before moving it in to a separate file
///// ｡･:*:･ﾟ★,｡･:*:･ﾟ☆　　 ｡･:*:･ﾟ★,｡･:*:･ﾟ☆
//// .｡*ﾟ+.*.｡　　　ﾟ+..｡*ﾟ+
/// .・。.・゜✭・.・✫・゜・。.
// 。°。°。°。°。°。°。°。°。°。°。°
༶•┈┈⛧┈♛ ♛┈⛧┈┈•༶
*/