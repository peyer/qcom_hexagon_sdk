local pak_utils = require 'p4://qctp406/source/qcom/qct/platform/adsp/buildtools/pakman/libs/pak_utils.lua'

local hex72 = {
  { "hexagon_Debug_dynamic_toolv72_v60", "hexagon_Release_dynamic_toolv72_v60", "hexagon_ReleaseG_dynamic_toolv72_v60", },
}
local hex74 = {
  { "hexagon_Debug_dynamic_toolv74", "hexagon_Release_dynamic_toolv74", "hexagon_ReleaseG_dynamic_toolv74", },
  { "v60", "v62"},
}
local hex80 = {
  { "hexagon_Debug_dynamic_toolv80", "hexagon_Release_dynamic_toolv80", "hexagon_ReleaseG_dynamic_toolv80", },
  { "v60", "v62"},
}
local hex81 = {
  { "hexagon_Debug_dynamic_toolv81", "hexagon_Release_dynamic_toolv81", "hexagon_ReleaseG_dynamic_toolv81", },
  { "v60", "v62", "v65"},
}
local hex82 = {
  { "hexagon_Debug_dynamic_toolv82", "hexagon_Release_dynamic_toolv82", "hexagon_ReleaseG_dynamic_toolv82", },
  { "v60", "v62", "v65", "v66"},
}

local hex83 = {
  { "hexagon_Debug_dynamic_toolv83", "hexagon_Release_dynamic_toolv83", "hexagon_ReleaseG_dynamic_toolv83", },
  { "v60", "v62", "v65", "v66"},
}

local hexagon_vs = {}
pak_utils.recur_variants(hexagon_vs, hex72)
pak_utils.recur_variants(hexagon_vs, hex74)
pak_utils.recur_variants(hexagon_vs, hex80)
pak_utils.recur_variants(hexagon_vs, hex81)
pak_utils.recur_variants(hexagon_vs, hex82)
pak_utils.recur_variants(hexagon_vs, hex83)

Vs = {}
for i,v in ipairs(hexagon_vs) do
  Vs[v] = v
end

return {
   supportedVs = Vs,
   defaultV = "hexagon_Debug_dynamic_toolv72_v60"
}

