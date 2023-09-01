
local pak_utils = require 'p4://qctp406/source/qcom/qct/platform/adsp/buildtools/pakman/libs/pak_utils.lua'

local android = {
  { "android_Debug", "android_Release", "android_ReleaseG", },
  { "aarch64", },
}

local hex81 = {
  { "hexagon_Debug_dynamic_toolv81_v65", "hexagon_Release_dynamic_toolv81_v65", "hexagon_ReleaseG_dynamic_toolv81_v65", },
}

local hex82 = {
  { "hexagon_Debug_dynamic_toolv82_v65", "hexagon_Release_dynamic_toolv82_v65", "hexagon_ReleaseG_dynamic_toolv82_v65",
    "hexagon_Debug_dynamic_toolv82_v66", "hexagon_Release_dynamic_toolv82_v66", "hexagon_ReleaseG_dynamic_toolv82_v66" },
}

local hex83 = {
  { "hexagon_Debug_dynamic_toolv83_v65", "hexagon_Release_dynamic_toolv83_v65", "hexagon_ReleaseG_dynamic_toolv83_v65",
    "hexagon_Debug_dynamic_toolv83_v66", "hexagon_Release_dynamic_toolv83_v66", "hexagon_ReleaseG_dynamic_toolv83_v66" },
}

local Vs_all = {}
pak_utils.recur_variants(Vs_all, hex81)
pak_utils.recur_variants(Vs_all, hex82)
pak_utils.recur_variants(Vs_all, hex83)
pak_utils.recur_variants(Vs_all, android)

Vs = {}
for i,v in ipairs(Vs_all) do
  Vs[v] = v
end

return {
   supportedVs = Vs,
   defaultV = "hexagon_Debug_dynamic_toolv83_v65"
}

