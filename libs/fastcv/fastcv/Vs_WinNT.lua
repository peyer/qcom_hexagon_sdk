local hexagon_vs = require 'p4://qctp406/source/qcom/qct/platform/adsp/buildtools/pakman/libs/default_hexagon_vs.lua'

Vs = {}
for i,v in ipairs(hexagon_vs) do
  Vs[v] = v
end

return {
   supportedVs = Vs,
   defaultV = "hexagon_Debug"
}
