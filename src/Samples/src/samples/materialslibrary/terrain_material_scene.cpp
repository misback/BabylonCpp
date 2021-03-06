#include <babylon/cameras/free_camera.h>
#include <babylon/interfaces/irenderable_scene.h>
#include <babylon/lights/hemispheric_light.h>
#include <babylon/materials/textures/texture.h>
#include <babylon/materialslibrary/terrain/terrain_material.h>
#include <babylon/meshes/ground_mesh.h>
#include <babylon/meshes/mesh.h>
#include <babylon/samples/babylon_register_sample.h>

namespace BABYLON {
namespace Samples {

/**
 * @brief Scene demonstrating the use of the terrain material from the materials library.
 * @see https://www.babylonjs-playground.com/#E6OZX#7
 * @see https://doc.babylonjs.com/extensions/Terrain
 */
struct TerrainMaterialScene : public IRenderableScene {

  TerrainMaterialScene(ICanvas* iCanvas) : IRenderableScene(iCanvas)
  {
  }

  ~TerrainMaterialScene() override = default;

  const char* getName() override
  {
    return "Terrain Material Scene";
  }

  void initializeScene(ICanvas* canvas, Scene* scene) override
  {
    // Camera
    auto camera = FreeCamera::New("camera1", Vector3(5.f, 4.f, -47.f), scene);
    camera->setTarget(Vector3::Zero());
    camera->attachControl(canvas, true);

    // Light
    auto light = HemisphericLight::New("light", Vector3(0.f, 1.f, 0.f), scene);

    // Create terrain material
    auto terrainMaterial = MaterialsLibrary::TerrainMaterial::New("terrainMaterial", scene);
    terrainMaterial->specularColor = Color3(0.5f, 0.5f, 0.5f);
    terrainMaterial->specularPower = 64.f;

    // Set the mix texture (represents the RGB values)
    terrainMaterial->mixTexture = Texture::New("textures/mixMap.png", scene);

    // Diffuse textures following the RGB values of the mix map
    // diffuseTexture1: Red
    // diffuseTexture2: Green
    // diffuseTexture3: Blue
    terrainMaterial->diffuseTexture1 = Texture::New("textures/floor.png", scene);
    terrainMaterial->diffuseTexture2 = Texture::New("textures/rock.png", scene);
    terrainMaterial->diffuseTexture3 = Texture::New("textures/grass.png", scene);

    // Bump textures according to the previously set diffuse textures
    terrainMaterial->bumpTexture1 = Texture::New("textures/floor_bump.png", scene);
    terrainMaterial->bumpTexture2 = Texture::New("textures/rockn.png", scene);
    terrainMaterial->bumpTexture3 = Texture::New("textures/grassn.png", scene);

    // Rescale textures according to the terrain
    terrainMaterial->diffuseTexture1()->uScale = terrainMaterial->diffuseTexture1()->vScale = 10.f;
    terrainMaterial->diffuseTexture2()->uScale = terrainMaterial->diffuseTexture2()->vScale = 10.f;
    terrainMaterial->diffuseTexture3()->uScale = terrainMaterial->diffuseTexture3()->vScale = 10.f;

    // Ground
    auto ground = Mesh::CreateGroundFromHeightMap("ground", "textures/heightMap.png", 100, 100, 100,
                                                  0, 10, scene, false);
    ground->position().y = -2.05f;
    ground->material     = terrainMaterial;
  }

}; // end of struct TerrainMaterialScene

BABYLON_REGISTER_SAMPLE("Materials Library", TerrainMaterialScene)

} // end of namespace Samples
} // end of namespace BABYLON
