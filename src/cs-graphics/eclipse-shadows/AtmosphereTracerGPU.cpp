////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AtmosphereTracerGPU.hpp"

#include "LUTPrecalculator.hpp"

#include "../../cs-utils/filesystem.hpp"
#include "Photon.hpp"

#include <GL/glew.h>
#include <iostream>

namespace cs::graphics {

void AtmosphereTracerGPU::init() {
  mLutPrecalculator = std::make_unique<LUTPrecalculator>();

  mProgram               = glCreateProgram();
  uint32_t computeShader = glCreateShader(GL_COMPUTE_SHADER);

  std::string code =
      cs::utils::filesystem::loadToString("../share/resources/shaders/EclipsePhotonTracer.glsl");
  const char* shader = code.c_str();
  glShaderSource(computeShader, 1, &shader, nullptr);
  glCompileShader(computeShader);

  int rvalue;
  glGetShaderiv(computeShader, GL_COMPILE_STATUS, &rvalue);
  if (!rvalue) {
    std::cerr << "Error in compiling the compute shader\n";
    GLchar  log[10240];
    GLsizei length;
    glGetShaderInfoLog(computeShader, 10239, &length, log);
    std::cerr << "Compiler log:\n" << log << std::endl;
    exit(40);
  }

  glAttachShader(mProgram, computeShader);
  glLinkProgram(mProgram);

  mUniforms.planetXPosition         = glGetUniformLocation(mProgram, "planet.xPosition");
  mUniforms.planetRadius            = glGetUniformLocation(mProgram, "planet.radius");
  mUniforms.planetAtmosphericHeight = glGetUniformLocation(mProgram, "planet.atmosphericHeight");
  mUniforms.planetSeaLevelMolecularNumberDensity =
      glGetUniformLocation(mProgram, "planet.seaLevelMolecularNumberDensity");

  mUniforms.pass     = glGetUniformLocation(mProgram, "pass");
  mUniforms.passSize = glGetUniformLocation(mProgram, "passSize");

  glDetachShader(mProgram, computeShader);
  glDeleteShader(computeShader);
}

AtmosphereTracerGPU::~AtmosphereTracerGPU() {
  glDeleteProgram(mProgram);
}

std::variant<GPUBuffer, CPUBuffer> AtmosphereTracerGPU::traceThroughAtmosphere(
    CPUBuffer const& photonBuffer, BodyWithAtmosphere const& body, double xPosition) {
  auto [ssboRefractiveIndices, ssboDensities] = mLutPrecalculator->createLUT(body);

  glUseProgram(mProgram);

  glUniform1d(mUniforms.planetXPosition, xPosition);
  glUniform1d(mUniforms.planetAtmosphericHeight, body.atmosphere.height);
  glUniform1d(mUniforms.planetSeaLevelMolecularNumberDensity,
      body.atmosphere.seaLevelMolecularNumberDensity);
  glUniform1d(mUniforms.planetRadius, body.meanRadius);

  uint32_t ssboPhotons;
  glGenBuffers(1, &ssboPhotons);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPhotons);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Photon) * photonBuffer.size(), photonBuffer.data(),
      GL_DYNAMIC_COPY);

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPhotons);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboRefractiveIndices);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboDensities);

  const uint32_t passSize   = 512u;
  const uint32_t maxPasses  = (photonBuffer.size() / passSize) + 1;
  const uint32_t numThreads = 32u;
  const uint32_t numBlocks  = passSize / numThreads;

  glUniform1ui(mUniforms.passSize, passSize);

  // Because the OS doesn't like long running GPU programs, we have to split them up. (╯°□°）╯︵ ┻━┻
  for (uint32_t pass = 0u; pass < maxPasses; ++pass) {
    glUniform1ui(mUniforms.pass, pass);

    glDispatchComputeGroupSizeARB(numBlocks, 1, 1, numThreads, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glFlush();
    glFinish();
  }
  /*
    std::vector<Photon> photons(numPhotons);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPhotons);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Photon) * photons.size(),
    photons.data());

    std::vector<glm::dvec3> positions(numPhotons);
    for (int i = 0; i < numPhotons; ++i) {
      positions[i] = (photons[i].position / 6371000.0) * 20.0;
    }

    auto objString = utils::verticesToObjString(positions);
    utils::filesystem::saveToFile(objString, "photon_positions_exit.obj");
  */
  glDeleteBuffers(1, &ssboDensities);
  glDeleteBuffers(1, &ssboRefractiveIndices);

  glUseProgram(0);

  return GPUBuffer{ssboPhotons, photonBuffer.size()};
}

} // namespace cs::graphics