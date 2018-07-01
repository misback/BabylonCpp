#include <babylon/gizmos/axis_drag_gizmo.h>

#include <babylon/babylon_stl_util.h>
#include <babylon/behaviors/mesh/pointer_drag_behavior.h>
#include <babylon/engine/scene.h>
#include <babylon/materials/standard_material.h>
#include <babylon/mesh/mesh_builder.h>
#include <babylon/mesh/vertex_data_options.h>
#include <babylon/rendering/utility_layer_renderer.h>

namespace BABYLON {

AxisDragGizmo::AxisDragGizmo(UtilityLayerRenderer* gizmoLayer,
                             const Vector3& dragAxis, const Color3& color)
    : Gizmo{gizmoLayer}
    , snapDistance{0.f}
    , _pointerObserver{nullptr}
    , _currentSnapDragDistance{0.f}
    , _tmpSnapEvent{0.f}
{
  // Create Material
  auto coloredMaterial
    = StandardMaterial::New("", gizmoLayer->utilityLayerScene.get());
  coloredMaterial->setDisableLighting(true);
  coloredMaterial->setEmissiveColor(color);

  auto hoverMaterial
    = StandardMaterial::New("", gizmoLayer->utilityLayerScene.get());
  hoverMaterial->setDisableLighting(true);
  hoverMaterial->setEmissiveColor(color.add(Color3(0.2f, 0.2f, 0.2f)));

  // Build mesh on root node
  auto arrow = AbstractMesh::New("", gizmoLayer->utilityLayerScene.get());
  CylinderOptions arrowMeshOptions;
  arrowMeshOptions.diameterTop  = 0.f;
  arrowMeshOptions.height       = 2.f;
  arrowMeshOptions.tessellation = 96;
  auto arrowMesh                = MeshBuilder::CreateCylinder(
    "yPosMesh", arrowMeshOptions, gizmoLayer->utilityLayerScene.get());
  CylinderOptions arrowTailOptions{0.015f};
  arrowTailOptions.height       = 0.3f;
  arrowTailOptions.tessellation = 96;
  auto arrowTail                = MeshBuilder::CreateCylinder(
    "yPosMesh", arrowTailOptions, gizmoLayer->utilityLayerScene.get());
  arrow->addChild(arrowMesh);
  arrow->addChild(arrowTail);

  // Position arrow pointing in its drag axis
  arrowMesh->scaling().scaleInPlace(0.05f);
  arrowMesh->material     = coloredMaterial;
  arrowMesh->rotation().x = Math::PI_2;
  arrowMesh->position().z += 0.3f;
  arrowTail->rotation().x = Math::PI_2;
  arrowTail->material     = coloredMaterial;
  arrowTail->position().z += 0.15f;
  arrow->lookAt(_rootMesh->position().subtract(dragAxis));

  _rootMesh->addChild(arrow);

  // Add drag behavior to handle events when the gizmo is dragged
  PointerDragBehaviorOptions options;
  options.dragAxis = dragAxis;
  // options.pointerObservableScene = gizmoLayer->originalScene;
  _dragBehavior = ::std::make_unique<PointerDragBehavior>(options);
  _dragBehavior->moveAttached = false;
  _rootMesh->addBehavior(_dragBehavior.get());
  _dragBehavior->onDragObservable.add([&](DragMoveEvent* event,
                                          EventState& /*es*/) {
    if (attachedMesh) {
      // Snapping logic
      if (snapDistance == 0.f) {
        attachedMesh()->position().addInPlace(event->delta);
      }
      else {
        _currentSnapDragDistance += event->dragDistance;
        if (::std::abs(_currentSnapDragDistance) > snapDistance) {
          auto dragSteps
            = ::std::floor(::std::abs(_currentSnapDragDistance) / snapDistance);
          _currentSnapDragDistance
            = std::fmod(_currentSnapDragDistance, snapDistance);
          event->delta.normalizeToRef(_tmpVector);
          _tmpVector.scaleInPlace(snapDistance * dragSteps);
          attachedMesh()->position().addInPlace(_tmpVector);
          _tmpSnapEvent.snapDistance = snapDistance * dragSteps;
          onSnapObservable.notifyObservers(&_tmpSnapEvent);
        }
      }
    }
  });

  _pointerObserver = gizmoLayer->utilityLayerScene->onPointerObservable.add(
    [&](PointerInfo* pointerInfo, EventState& /*es*/) {
      if (stl_util::contains(_rootMesh->getChildMeshes(),
                             pointerInfo->pickInfo.pickedMesh)) {
        for (auto& m : _rootMesh->getChildMeshes()) {
          m->material = hoverMaterial;
        }
      }
      else {
        for (auto& m : _rootMesh->getChildMeshes()) {
          m->material = coloredMaterial;
        }
      }
    });
}

AxisDragGizmo::~AxisDragGizmo()
{
}

void AxisDragGizmo::_attachedMeshChanged(AbstractMesh* value)
{
  if (_dragBehavior) {
    _dragBehavior->enabled = value ? true : false;
  }
}

void AxisDragGizmo::dispose(bool doNotRecurse, bool disposeMaterialAndTextures)
{
  onSnapObservable.clear();
  gizmoLayer->utilityLayerScene->onPointerObservable.remove(_pointerObserver);
  _dragBehavior->detach();
  Gizmo::dispose(doNotRecurse, disposeMaterialAndTextures);
}

} // end of namespace BABYLON
