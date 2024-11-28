//
// Created by cezary on 28.11.24.
//

#include "tr_shadercompiler.hpp"

#include "tr_logger.hpp"
#include "tr_appconfig.hpp"

#include <filesystem>
#include <set>
#include <fstream>
#include <cstring>
#include <array>

#include <shaderc/shaderc.hpp>

#include <vulkan/vulkan.hpp>

using namespace std;

namespace shaders{

    bool replace(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = str.find(from);
        if(start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    void UpdateCommonHeaderValues(){
        string commonHeader = SHADER_SOURCES_DIRECTORY;
        commonHeader += "/common.h";

        std::ifstream inputfile( commonHeader);

        std::string content(
                (std::istreambuf_iterator<char>(inputfile)),
                (std::istreambuf_iterator<char>()));

        inputfile.close();

        /*
         *
         #define CMN_MAX_PREFERRED_MESH_WORK_GROUP_INVOCATIONS $m_MaxPreferredMeshWorkGroupInvocations
#define CMN_MAX_MESH_OUTPUT_VERTICES $m_MaxMeshOutputVertices
#define CMN_MAX_MESH_OUTPUT_PRIMITIVES $m_MaxMeshOutputPrimitives

         */
        // Replace define values with appropriate from appconfig

        replace(content, "$m_MaxPreferredMeshWorkGroupInvocations", to_string(APP_CONFIG.m_MeshShaderConfig.m_MaxPreferredMeshWorkGroupInvocations));
        replace(content, "$m_MaxMeshOutputVertices", to_string(APP_CONFIG.m_MeshShaderConfig.m_MaxMeshOutputVertices));
        replace(content, "$m_MaxMeshOutputPrimitives", to_string(APP_CONFIG.m_MeshShaderConfig.m_MaxMeshOutputPrimitives));


        // write content back to header
        std::ofstream fs(commonHeader, std::ios::out);
        fs << content;
        fs.close();
    }

    class NEShaderIncluder : public shaderc::CompileOptions::IncluderInterface
    {
        shaderc_include_result* GetInclude(
                const char* requested_source,
                shaderc_include_type type,
                const char* requesting_source,
                size_t include_depth)
        {
            const string name = string(requested_source);
            const string contents;

            std::string inputFilePath = SHADER_SOURCES_DIRECTORY;
            inputFilePath += "/";
            inputFilePath += string(requested_source);
            std::ifstream inputfile( inputFilePath);

            std::string content(
                    (std::istreambuf_iterator<char>(inputfile)),
                    (std::istreambuf_iterator<char>()));

            inputfile.close();

            auto container = new std::array<std::string, 2>;
            (*container)[0] = name;
            (*container)[1] = contents;

            auto data = new shaderc_include_result;

            data->user_data = container;

            data->source_name = (*container)[0].data();
            data->source_name_length = (*container)[0].size();

            data->content = (*container)[1].data();
            data->content_length = (*container)[1].size();

            return data;
        };

        void ReleaseInclude(shaderc_include_result* data) override
        {
            delete static_cast<std::array<std::string, 2>*>(data->user_data);
            delete data;
        };
    };


    const std::set<std::string> SUPPORTED_SHADER_EXTENSIONS = { ".frag", ".mesh", ".task" };

    uint32_t ReverseBytes(uint32_t value)
    {
        return (value & 0x000000FF) << 24 | (value & 0x0000FF00) << 8 |
               (value & 0x00FF0000) >> 8 | (value & 0xFF000000) >> 24;
    }

    void compileShader(std::filesystem::path filePath, std::filesystem::path outputDirectory){

        log("Compiling " + filePath.filename().string());

        // Setup
        shaderc_shader_kind shaderKind;

        if(filePath.extension() == ".mesh")
            shaderKind = shaderc_shader_kind::shaderc_glsl_mesh_shader;
        else if(filePath.extension() == ".frag")
            shaderKind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
        else
            shaderKind = shaderc_shader_kind::shaderc_glsl_task_shader;

        // read source code
        std::ifstream inputfile(filePath);

        std::string content(
                (std::istreambuf_iterator<char>(inputfile)),
                (std::istreambuf_iterator<char>()));

        inputfile.close();

        shaderc::CompileOptions compileOptions;
        compileOptions.SetTargetSpirv(shaderc_spirv_version_1_5);
        compileOptions.SetIncluder(std::make_unique<NEShaderIncluder>());
        compileOptions.SetTargetEnvironment(shaderc_target_env::shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);


        // Preprocess
        shaderc::Compiler compiler;
        shaderc::PreprocessedSourceCompilationResult preprocessedSourceCompilationResult = compiler.PreprocessGlsl(content, shaderKind, filePath.filename().c_str(), compileOptions);

        if(preprocessedSourceCompilationResult.GetCompilationStatus() != shaderc_compilation_status_success){
            logError(preprocessedSourceCompilationResult.GetErrorMessage());
        }

        const char *src = preprocessedSourceCompilationResult.cbegin();
        size_t newSize = preprocessedSourceCompilationResult.cend() - src;
        content.resize(newSize);
        memcpy(content.data(), src, newSize);

//         log("Preprocessed source:");
//         log(content);


        // Compilation to spir-v assembly
        shaderc::AssemblyCompilationResult assemblyCompileResult = compiler.CompileGlslToSpvAssembly(content, shaderKind, filePath.filename().c_str(), compileOptions);

        if(assemblyCompileResult.GetCompilationStatus() != shaderc_compilation_status_success){
            logError(assemblyCompileResult.GetErrorMessage());
        }

        src = assemblyCompileResult.cbegin();
        newSize = assemblyCompileResult.cend() - src;
        content.resize(newSize);
        memcpy(content.data(), src, newSize);

//        log("SPIR-V assembly:");
//        log(content);



        // Compilation to binary
        shaderc::CompilationResult binaryCompilationResult = compiler.AssembleToSpv(content, compileOptions);

        if(binaryCompilationResult.GetCompilationStatus() != shaderc_compilation_status_success){
            logError(binaryCompilationResult.GetErrorMessage());
        }

        const uint32_t *binarySrc = binaryCompilationResult.cbegin();
        newSize = binaryCompilationResult.cend() - binarySrc;
        std::cout << "Size of binary: " << newSize << endl;
        vector<uint32_t> binaryContent(newSize);

        memcpy(binaryContent.data(), binarySrc, newSize * sizeof(uint32_t));

        string outputFilePath = outputDirectory.string() + "/";
        outputFilePath += filePath.filename().string() + ".spv";

        std::ofstream fs(outputFilePath, std::ios::out | std::ios::binary);
        fs.write((char *)(binaryContent.data()), newSize * sizeof(uint32_t));
        fs.close();
    }

    void CompileShaders(){

#ifndef SHADER_SOURCES_DIRECTORY
        logError("SHADER_SOURCES_DIRECTORY undefined. Cannot find shader sources otherwise!!!");
        exit(1);
#elifndef SHADER_BINARIES_DIRECTORY
        logError("SHADER_BINARIES_DIRECTORY undefined. Cannot build shader binaries to no directory!!!");
        exit(1);
#endif

        UpdateCommonHeaderValues();

        std::string shaderSourcesDirectory = SHADER_SOURCES_DIRECTORY;
        std::string shaderBinariesDirectory = SHADER_BINARIES_DIRECTORY;

        for(auto f : std::filesystem::directory_iterator(shaderSourcesDirectory)){

            if(!f.is_directory() && SUPPORTED_SHADER_EXTENSIONS.count(f.path().extension())){
                compileShader(f.path(), shaderBinariesDirectory);
            }
        }
    }
}