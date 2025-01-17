/// SPDX-License-Identifier: MIT
#include "ashura/engine/shader.h"
#include "SPIRV/GlslangToSpv.h"
#include "ashura/std/vec.h"
#include "glslang/Public/ShaderLang.h"

namespace ash
{

constexpr TBuiltInResource SHADER_RESOURCE_LIMITS = {
  .maxLights                                 = 32,
  .maxClipPlanes                             = 6,
  .maxTextureUnits                           = 32,
  .maxTextureCoords                          = 32,
  .maxVertexAttribs                          = 64,
  .maxVertexUniformComponents                = 4'096,
  .maxVaryingFloats                          = 64,
  .maxVertexTextureImageUnits                = 32,
  .maxCombinedTextureImageUnits              = 80,
  .maxTextureImageUnits                      = 32,
  .maxFragmentUniformComponents              = 4'096,
  .maxDrawBuffers                            = 32,
  .maxVertexUniformVectors                   = 128,
  .maxVaryingVectors                         = 8,
  .maxFragmentUniformVectors                 = 16,
  .maxVertexOutputVectors                    = 16,
  .maxFragmentInputVectors                   = 15,
  .minProgramTexelOffset                     = -8,
  .maxProgramTexelOffset                     = 7,
  .maxClipDistances                          = 8,
  .maxComputeWorkGroupCountX                 = 65'535,
  .maxComputeWorkGroupCountY                 = 65'535,
  .maxComputeWorkGroupCountZ                 = 65'535,
  .maxComputeWorkGroupSizeX                  = 1'024,
  .maxComputeWorkGroupSizeY                  = 1'024,
  .maxComputeWorkGroupSizeZ                  = 64,
  .maxComputeUniformComponents               = 1'024,
  .maxComputeTextureImageUnits               = 16,
  .maxComputeImageUniforms                   = 8,
  .maxComputeAtomicCounters                  = 8,
  .maxComputeAtomicCounterBuffers            = 1,
  .maxVaryingComponents                      = 60,
  .maxVertexOutputComponents                 = 64,
  .maxGeometryInputComponents                = 64,
  .maxGeometryOutputComponents               = 128,
  .maxFragmentInputComponents                = 128,
  .maxImageUnits                             = 8,
  .maxCombinedImageUnitsAndFragmentOutputs   = 8,
  .maxCombinedShaderOutputResources          = 8,
  .maxImageSamples                           = 0,
  .maxVertexImageUniforms                    = 0,
  .maxTessControlImageUniforms               = 0,
  .maxTessEvaluationImageUniforms            = 0,
  .maxGeometryImageUniforms                  = 0,
  .maxFragmentImageUniforms                  = 8,
  .maxCombinedImageUniforms                  = 8,
  .maxGeometryTextureImageUnits              = 16,
  .maxGeometryOutputVertices                 = 256,
  .maxGeometryTotalOutputComponents          = 1'024,
  .maxGeometryUniformComponents              = 1'024,
  .maxGeometryVaryingComponents              = 64,
  .maxTessControlInputComponents             = 128,
  .maxTessControlOutputComponents            = 128,
  .maxTessControlTextureImageUnits           = 16,
  .maxTessControlUniformComponents           = 1'024,
  .maxTessControlTotalOutputComponents       = 4'096,
  .maxTessEvaluationInputComponents          = 128,
  .maxTessEvaluationOutputComponents         = 128,
  .maxTessEvaluationTextureImageUnits        = 16,
  .maxTessEvaluationUniformComponents        = 1'024,
  .maxTessPatchComponents                    = 120,
  .maxPatchVertices                          = 32,
  .maxTessGenLevel                           = 64,
  .maxViewports                              = 16,
  .maxVertexAtomicCounters                   = 0,
  .maxTessControlAtomicCounters              = 0,
  .maxTessEvaluationAtomicCounters           = 0,
  .maxGeometryAtomicCounters                 = 0,
  .maxFragmentAtomicCounters                 = 8,
  .maxCombinedAtomicCounters                 = 8,
  .maxAtomicCounterBindings                  = 1,
  .maxVertexAtomicCounterBuffers             = 0,
  .maxTessControlAtomicCounterBuffers        = 0,
  .maxTessEvaluationAtomicCounterBuffers     = 0,
  .maxGeometryAtomicCounterBuffers           = 0,
  .maxFragmentAtomicCounterBuffers           = 1,
  .maxCombinedAtomicCounterBuffers           = 1,
  .maxAtomicCounterBufferSize                = 16'384,
  .maxTransformFeedbackBuffers               = 4,
  .maxTransformFeedbackInterleavedComponents = 64,
  .maxCullDistances                          = 8,
  .maxCombinedClipAndCullDistances           = 8,
  .maxSamples                                = 4,
  .maxMeshOutputVerticesNV                   = 256,
  .maxMeshOutputPrimitivesNV                 = 512,
  .maxMeshWorkGroupSizeX_NV                  = 32,
  .maxMeshWorkGroupSizeY_NV                  = 1,
  .maxMeshWorkGroupSizeZ_NV                  = 1,
  .maxTaskWorkGroupSizeX_NV                  = 32,
  .maxTaskWorkGroupSizeY_NV                  = 1,
  .maxTaskWorkGroupSizeZ_NV                  = 1,
  .maxMeshViewCountNV                        = 4,
  .maxMeshOutputVerticesEXT                  = 256,
  .maxMeshOutputPrimitivesEXT                = 256,
  .maxMeshWorkGroupSizeX_EXT                 = 128,
  .maxMeshWorkGroupSizeY_EXT                 = 128,
  .maxMeshWorkGroupSizeZ_EXT                 = 128,
  .maxTaskWorkGroupSizeX_EXT                 = 128,
  .maxTaskWorkGroupSizeY_EXT                 = 128,
  .maxTaskWorkGroupSizeZ_EXT                 = 128,
  .maxMeshViewCountEXT                       = 4,
  .maxDualSourceDrawBuffersEXT               = 1,
  .limits{
          .nonInductiveForLoops                 = true,
          .whileLoops                           = true,
          .doWhileLoops                         = true,
          .generalUniformIndexing               = true,
          .generalAttributeMatrixVectorIndexing = true,
          .generalVaryingIndexing               = true,
          .generalSamplerIndexing               = true,
          .generalVariableIndexing              = true,
          .generalConstantMatrixVectorIndexing  = true,
          }
};

struct Includer : glslang::TShader::Includer
{
  ShaderCompileInfo info;
  AllocatorRef      allocator;

  Includer(ShaderCompileInfo info, AllocatorRef allocator) :
    info{info},
    allocator{allocator}
  {
  }

  Includer(Includer const &)             = default;
  Includer(Includer &&)                  = default;
  Includer & operator=(Includer const &) = default;
  Includer & operator=(Includer &&)      = default;
  virtual ~Includer() override           = default;

  virtual IncludeResult * includeLocal(char const * header_name,
                                       char const * includer_name,
                                       size_t       inclusion_depth) override
  {
    (void) includer_name;
    (void) inclusion_depth;

    return info.on_load(Span{header_name, strlen(header_name)})
      .match(
        [&](Span<char const> header_data) -> IncludeResult * {
          IncludeResult * result;
          if (!allocator->nalloc(1, result))
          {
            info.on_log(LogLevel::Error,
                        "Failed to allocate memory for Include Result");
            return nullptr;
          }

          new (result) IncludeResult{header_name, header_data.data(),
                                     header_data.size(), nullptr};

          return result;
        },
        [&]() -> IncludeResult * { return nullptr; });
  }

  virtual IncludeResult * includeSystem(char const * header_name,
                                        char const * includer_name,
                                        size_t       inclusion_depth) override
  {
    return includeLocal(header_name, includer_name, inclusion_depth);
  }

  virtual void releaseInclude(IncludeResult * inc) override
  {
    if (inc != nullptr)
    {
      info.on_drop(inc->headerName);
      inc->~IncludeResult();
      allocator->ndealloc(1, inc);
    }
  }
};

Result<Void, ShaderLoadErr> compile_shader(ShaderCompileInfo const & info,
                                           Vec<u32> &                spirv,
                                           AllocatorRef              allocator)
{
  if (!glslang::InitializeProcess())
  {
    return Err{ShaderLoadErr::InitErr};
  }

  defer glsl_([] { glslang::FinalizeProcess(); });

  EShLanguage language = EShLanguage::EShLangVertex;

  switch (info.type)
  {
    case ShaderType::Compute:
      language = EShLanguage::EShLangCompute;
      break;
    case ShaderType::Fragment:
      language = EShLanguage::EShLangFragment;
      break;
    case ShaderType::Mesh:
      language = EShLanguage::EShLangMesh;
      break;
    case ShaderType::Vertex:
      language = EShLanguage::EShLangVertex;
      break;
    default:
      CHECK_UNREACHABLE();
  }

  Option buff = info.on_load(info.path);

  if (!buff)
  {
    return Err{ShaderLoadErr::IOErr};
  }

  defer unload{[&]() { info.on_drop(info.path); }};

  CHECK(buff.value().size() <= I32_MAX);

  char const * buff_p      = (char *) buff.value().data();
  int          buff_length = (int) buff.value().size32();

  glslang::TShader shader{language};
  shader.setStringsWithLengths(&buff_p, &buff_length, 1);
  shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientVulkan,
                     100);
  shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
  shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

  auto check_log = [&](auto & object) {
    char const * info_log  = object.getInfoLog();
    usize const  info_len  = info_log == nullptr ? 0 : strlen(info_log);
    char const * debug_log = object.getInfoDebugLog();
    usize const  debug_len = debug_log == nullptr ? 0 : strlen(debug_log);

    if (info_len != 0)
    {
      info.on_log(LogLevel::Info, Span{info_log, info_len});
    }

    if (debug_len != 0)
    {
      info.on_log(LogLevel::Debug, Span{debug_log, debug_len});
    }
  };

  constexpr char const * entry_point = "main";

  shader.setEntryPoint(entry_point);
  shader.setSourceEntryPoint(entry_point);

  defer shader_log_{[&] { check_log(shader); }};

  Vec<char> preamble{allocator};

  if (!preamble.extend(info.preamble))
  {
    return Err{ShaderLoadErr::OutOfMemory};
  }

  if (!preamble.extend(span({'\n', '\0'})))
  {
    return Err{ShaderLoadErr::OutOfMemory};
  }

  shader.setPreamble(preamble.data());

  Includer includer{info, allocator};

  if (!shader.parse(&SHADER_RESOURCE_LIMITS, 100, false, EShMsgDefault,
                    includer))
  {
    return Err{ShaderLoadErr::CompileFailed};
  }

  glslang::TProgram program;
  program.addShader(&shader);

  defer program_log_{[&] { check_log(program); }};

  if (!program.link(EShMsgDefault))
  {
    return Err{ShaderLoadErr::LinkFailed};
  }

  glslang::TIntermediate * intermediate = program.getIntermediate(language);

  if (intermediate == nullptr)
  {
    return Err{ShaderLoadErr::LinkFailed};
  }

  glslang::SpvOptions spvOptions{.generateDebugInfo                = true,
                                 .stripDebugInfo                   = false,
                                 .disableOptimizer                 = false,
                                 .optimizeSize                     = true,
                                 .disassemble                      = false,
                                 .validate                         = true,
                                 .emitNonSemanticShaderDebugInfo   = false,
                                 .emitNonSemanticShaderDebugSource = false,
                                 .compileOnly                      = false};

  spv::SpvBuildLogger spv_logger;

  std::vector<u32> spirv_v;
  glslang::GlslangToSpv(*intermediate, spirv_v, &spv_logger, &spvOptions);
  std::string conv_messages = spv_logger.getAllMessages();

  if (!conv_messages.empty())
  {
    info.on_log(LogLevel::Info, conv_messages);
  }

  if (!spirv.extend(spirv_v))
  {
    return Err{ShaderLoadErr::OutOfMemory};
  }

  return Ok{};
}

}    // namespace ash
