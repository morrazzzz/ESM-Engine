#pragma once
#include "script_export_space.h"

using CScriptRandom = class_exporter<CRandom>;
add_to_type_list(CScriptRandom)
#undef script_type_list
#define script_type_list save_type_list(CScriptRandom)