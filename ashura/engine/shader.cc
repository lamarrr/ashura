#include "ashura/engine/shader.h"
#include "ashura/std/io.h"
#include "ashura/std/vec.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/SPIRV/spirv.hpp"
#include "glslang/SPIRV/spvIR.h"
#include "spirv_reflect.h"
#include <filesystem>

namespace ash
{

constexpr TBuiltInResource SHADER_RESOURCE_LIMITS = {
    .maxLights                                 = 32,
    .maxClipPlanes                             = 6,
    .maxTextureUnits                           = 32,
    .maxTextureCoords                          = 32,
    .maxVertexAttribs                          = 64,
    .maxVertexUniformComponents                = 4096,
    .maxVaryingFloats                          = 64,
    .maxVertexTextureImageUnits                = 32,
    .maxCombinedTextureImageUnits              = 80,
    .maxTextureImageUnits                      = 32,
    .maxFragmentUniformComponents              = 4096,
    .maxDrawBuffers                            = 32,
    .maxVertexUniformVectors                   = 128,
    .maxVaryingVectors                         = 8,
    .maxFragmentUniformVectors                 = 16,
    .maxVertexOutputVectors                    = 16,
    .maxFragmentInputVectors                   = 15,
    .minProgramTexelOffset                     = -8,
    .maxProgramTexelOffset                     = 7,
    .maxClipDistances                          = 8,
    .maxComputeWorkGroupCountX                 = 65535,
    .maxComputeWorkGroupCountY                 = 65535,
    .maxComputeWorkGroupCountZ                 = 65535,
    .maxComputeWorkGroupSizeX                  = 1024,
    .maxComputeWorkGroupSizeY                  = 1024,
    .maxComputeWorkGroupSizeZ                  = 64,
    .maxComputeUniformComponents               = 1024,
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
    .maxGeometryTotalOutputComponents          = 1024,
    .maxGeometryUniformComponents              = 1024,
    .maxGeometryVaryingComponents              = 64,
    .maxTessControlInputComponents             = 128,
    .maxTessControlOutputComponents            = 128,
    .maxTessControlTextureImageUnits           = 16,
    .maxTessControlUniformComponents           = 1024,
    .maxTessControlTotalOutputComponents       = 4096,
    .maxTessEvaluationInputComponents          = 128,
    .maxTessEvaluationOutputComponents         = 128,
    .maxTessEvaluationTextureImageUnits        = 16,
    .maxTessEvaluationUniformComponents        = 1024,
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
    .maxAtomicCounterBufferSize                = 16384,
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
    .limits                                    = {
                                           .nonInductiveForLoops                 = true,
                                           .whileLoops                           = true,
                                           .doWhileLoops                         = true,
                                           .generalUniformIndexing               = true,
                                           .generalAttributeMatrixVectorIndexing = true,
                                           .generalVaryingIndexing               = true,
                                           .generalSamplerIndexing               = true,
                                           .generalVariableIndexing              = true,
                                           .generalConstantMatrixVectorIndexing  = true,
    }};

struct Includer : glslang::TShader::Includer
{
  Includer()
  {
  }

  Includer(Includer const &)            = delete;
  Includer(Includer &&)                 = delete;
  Includer &operator=(Includer const &) = delete;
  Includer &operator=(Includer &&)      = delete;

  virtual IncludeResult *includeLocal(const char *header_name,
                                      const char *includer_name,
                                      size_t      inclusion_depth) override
  {
    return include_path(header_name, includer_name, (int) inclusion_depth,
                        true);
  }

  virtual IncludeResult *includeSystem(const char *header_name,
                                       const char *includer_name,
                                       size_t      inclusion_depth) override
  {
    return include_path(header_name, includer_name, (int) inclusion_depth,
                        false);
  }

  virtual void releaseInclude(IncludeResult *result) override
  {
    if (result != nullptr)
    {
      Vec<char> *blob = (Vec<char> *) result->userData;
      blob->reset();
      default_allocator.deallocate_typed(blob, 1);
      result->~IncludeResult();
      default_allocator.deallocate_typed(result, 1);
    }
  }

  virtual ~Includer() override
  {
  }

  IncludeResult *include_path(const char *header_name,
                              const char *includer_name, int depth,
                              bool is_local)
  {
    IncludeResult *result = nullptr;
    if (is_local)
    {
      result = include_local(header_name, includer_name, depth);
      if (result != nullptr)
      {
        return result;
      }
      return include_system(header_name, includer_name, depth);
    }

    result = include_system(header_name, includer_name, depth);
    if (result != nullptr)
    {
      return result;
    }
    return include_local(header_name, includer_name, depth);
  }

  IncludeResult *include_local(const char *header_name,
                               const char *includer_name, int depth)
  {
    (void) includer_name;
    (void) depth;
    {
      std::filesystem::path first_cnd{includer_name};
      first_cnd /= header_name;

      if (std::filesystem::exists(first_cnd))
      {
        std::string p = first_cnd.string();
        return include_file(header_name, p.c_str());
      }
    }

    for (Span folder : local_directories)
    {
      std::filesystem::path cnd{std::string_view{folder.data(), folder.size()}};
      cnd /= header_name;

      if (std::filesystem::exists(cnd))
      {
        std::string p = cnd.string();
        return include_file(header_name, p.c_str());
      }
    }

    return nullptr;
  }

  IncludeResult *include_system(const char *header_name,
                                const char *includer_name, int depth)
  {
    (void) includer_name;
    (void) depth;
    for (Span folder : system_directories)
    {
      std::filesystem::path cnd{std::string_view{folder.data(), folder.size()}};
      cnd /= header_name;

      if (std::filesystem::exists(cnd))
      {
        std::string p = cnd.string();
        return include_file(header_name, p.c_str());
      }
    }
    return nullptr;
  }

  IncludeResult *include_file(char const *header_name, char const *path)
  {
    Vec<u8> *blob = default_allocator.allocate_typed<Vec<u8>>(1);
    if (blob == nullptr)
    {
      return nullptr;
    }
    new (blob) Vec<u8>{};
    defer blob_del{[&] {
      if (blob != nullptr)
      {
        blob->reset();
        default_allocator.deallocate_typed(blob, 1);
      }
    }};

    if (read_file(Span{path, strlen(path)}, *blob) != IoError::None)
    {
      return nullptr;
    }

    IncludeResult *result = default_allocator.allocate_typed<IncludeResult>(1);
    if (result == nullptr)
    {
      return nullptr;
    }

    new (result)
        IncludeResult{header_name, (char *) blob->data(), blob->size(), blob};
    blob = nullptr;

    return result;
  }

  Span<Span<char const> const> system_directories;
  Span<Span<char const> const> local_directories;
};

void reflect_spirv(Span<u32 const> spirv)
{
  // Generate reflection data for a shader
  SpvReflectShaderModule module;
  SpvReflectResult       result = spvReflectCreateShaderModule(
      spirv.size() * sizeof(u32), spirv.data(), &module);
  CHECK(result == SPV_REFLECT_RESULT_SUCCESS);
  defer m_destroy{[&] { spvReflectDestroyShaderModule(&module); }};

  // Enumerate and extract shader's input variables
  u32 var_count = 0;
  result        = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
  CHECK(result == SPV_REFLECT_RESULT_SUCCESS);
  SpvReflectInterfaceVariable **input_vars =
      (SpvReflectInterfaceVariable **) malloc(
          var_count * sizeof(SpvReflectInterfaceVariable *));
  defer iv_close{[&] { free(input_vars); }};
  result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars);
  CHECK(result == SPV_REFLECT_RESULT_SUCCESS);

  // spvReflectEnumeratePushConstants
  // spvReflectEnumerateDescriptorBindings
  //
  // Output variables, descriptor bindings, descriptor sets, and push constants
  // can be enumerated and extracted using a similar mechanism.
  //
  // Destroy the reflection data when no longer required.
}

ShaderCompileError
    compile_shader(Logger &logger, Vec<u32> &spirv, Span<char const> file,
                   ShaderType type, Span<char const> preamble,
                   Span<char const>             entry_point,
                   Span<Span<char const> const> system_directories,
                   Span<Span<char const> const> local_directories)
{
  CHECK(glslang::InitializeProcess());
  defer       glsl_del([] { glslang::FinalizeProcess(); });
  EShLanguage language = EShLanguage::EShLangVertex;

  Vec<u8> buff;
  defer   buff_del{[&] { buff.reset(); }};

  IoError io_err = read_file(file, buff);
  if (io_err != IoError::None)
  {
    return ShaderCompileError::IOError;
  }

  switch (type)
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
      UNREACHABLE();
  }

  char const      *buff_p      = (char *) buff.data();
  int              buff_length = (int) buff.size();
  glslang::TShader shader{language};
  shader.setStringsWithLengths(&buff_p, &buff_length, 1);
  shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientVulkan,
                     100);
  shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
  shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

  Vec<char> entry_point_s;
  defer     entry_point_del{[&] { entry_point_s.reset(); }};
  if (!entry_point_s.extend_copy(entry_point))
  {
    return ShaderCompileError::OutOfMemory;
  }
  if (!entry_point_s.push((char) 0))
  {
    return ShaderCompileError::OutOfMemory;
  }
  shader.setEntryPoint(entry_point_s.data());
  shader.setSourceEntryPoint(entry_point_s.data());
  Vec<char> preamble_s;
  defer     preamble_del{[&] { preamble_s.reset(); }};
  if (!preamble_s.extend_copy(preamble))
  {
    return ShaderCompileError::OutOfMemory;
  }
  if (!preamble_s.extend_copy(to_span({'\n', '\0'})))
  {
    return ShaderCompileError::OutOfMemory;
  }
  shader.setPreamble(preamble_s.data());

  Includer includer;
  includer.system_directories = system_directories;
  includer.local_directories  = local_directories;

  // expand to source
  std::string str;
  if (!shader.preprocess(&SHADER_RESOURCE_LIMITS, 100, ENoProfile, false, false,
                         EShMsgDefault, &str, includer))
  {
    logger.error(shader.getInfoLog(), "\n", shader.getInfoDebugLog());
    return ShaderCompileError::CompileFailed;
  }

  if (!shader.parse(&SHADER_RESOURCE_LIMITS, 100, false, EShMsgDefault,
                    includer))
  {
    logger.error(shader.getInfoLog(), "\n", shader.getInfoDebugLog());
    return ShaderCompileError::CompileFailed;
  }

  glslang::TProgram program;
  program.addShader(&shader);
  if (!program.link(EShMsgDefault))
  {
    return ShaderCompileError::LinkFailed;
  }

  glslang::TIntermediate *intermediate = program.getIntermediate(language);
  if (intermediate == nullptr)
  {
    return ShaderCompileError::LinkFailed;
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
    logger.warn(conv_messages);
  }

  if (!spirv.extend_copy(to_span(spirv_v)))
  {
    return ShaderCompileError::OutOfMemory;
  }

  return ShaderCompileError::None;
}

}        // namespace ash