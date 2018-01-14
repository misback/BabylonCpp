#ifndef BABYLON_MATERIALS_TEXTURES_PROCEDURALS_CUSTOM_PROCEDURAL_TEXTURE_H
#define BABYLON_MATERIALS_TEXTURES_PROCEDURALS_CUSTOM_PROCEDURAL_TEXTURE_H

#include <babylon/babylon_global.h>
#include <babylon/core/json.h>
#include <babylon/materials/textures/procedurals/procedural_texture.h>

namespace BABYLON {

class BABYLON_SHARED_EXPORT CustomProceduralTexture : public ProceduralTexture {

public:
  CustomProceduralTexture(const string_t& name, const string_t& texturePath,
                          int size, Scene* scene,
                          Texture* fallbackTexture = nullptr,
                          bool generateMipMaps     = false);
  ~CustomProceduralTexture();

  bool isReady() override;
  void render(bool useCameraPostProcess = false);
  void updateTextures();
  void updateShaderUniforms();
  bool animate() const;
  void setAnimate(bool value);

private:
  void loadJson(const string_t& jsonUrl);

private:
  bool _animate;
  float _time;
  bool _configSet;
  Json::value _config;
  string_t _texturePath;

}; // end of class CustomProceduralTexture

} // end of namespace BABYLON

#endif // end of
       // BABYLON_MATERIALS_TEXTURES_PROCEDURALS_CUSTOM_PROCEDURAL_TEXTURE_H
