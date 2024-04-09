HOW TO STRUCTURE SHADER PACKS
SPIRV will be the result of editor shader compilation

ALL shaders are compiled to a pack with spirv


we load these pre-compiled SPIRVs at load-time for dseployed builds, and
for editior we load once changed


other shaders should be able to include their own code and our default
library code


editor combines shader nodes into shaders
at editor build time, the shaders are then compiled into spirv along and
tagged. this goes into a shader pack with spirv code and ids load spirv and
ids at runtime
