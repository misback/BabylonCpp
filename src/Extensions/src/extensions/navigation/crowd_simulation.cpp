#include <babylon/extensions/navigation/crowd_simulation.h>

#include <babylon/culling/bounding_box.h>
#include <babylon/culling/bounding_info.h>
#include <babylon/extensions/navigation/rvo2/rvo_simulator.h>
#include <babylon/mesh/abstract_mesh.h>

namespace BABYLON {
namespace Extensions {

CrowdSimulation::CrowdSimulation()
    : _simulator{::std::make_unique<RVO2::RVOSimulator>()}
    , _crowdCollisionAvoidanceSystem{
        CrowdCollisionAvoidanceSystem(_simulator.get())}
{
  initializeWorld();
  _simulator->setAgentDefaults(15.0f, 10, 5.0f, 5.0f, 2.0f, 2.0f);
}

CrowdSimulation::~CrowdSimulation()
{
}

void CrowdSimulation::initializeWorld()
{
  _world.addSystem(_crowdCollisionAvoidanceSystem);
  _world.addSystem(_crowdMeshUpdaterSystem);
}

void CrowdSimulation::setTimeStep(float timeStep)
{
  _simulator->setTimeStep(timeStep);
}

size_t CrowdSimulation::addAgent(AbstractMesh* mesh)
{
  BABYLON::Vector2 position(mesh->position().x, mesh->position().z);
  return addAgent(mesh, position);
}

size_t CrowdSimulation::addAgent(AbstractMesh* mesh,
                                 const BABYLON::Vector2& position)
{
  // Create the crowd agent entity
  auto agent = _world.createEntity();

  // Set the agent's mesh
  auto& agentMesh = agent.addComponent<CrowdMesh>().mesh;
  agentMesh       = mesh;

  // Set the agent's properties
  auto& agentComp = agent.addComponent<CrowdAgent>(_simulator.get(), position);

  // Set the agent radius
  const auto& bbox  = mesh->getBoundingInfo()->boundingBox;
  const auto& min   = bbox.minimumWorld;
  const auto& max   = bbox.maximumWorld;
  const auto box    = max.subtract(min).scale(0.5f);
  const auto radius = (box.x + box.z) * 0.5f;
  agentComp.setRadius(radius);

  // Activate and store the crowd agent entity
  agent.activate();
  _agents.emplace_back(agent);

  return agentComp.id();
}

void CrowdSimulation::setAgentGoal(size_t agentId, const BABYLON::Vector2& goal)
{
  _agents[agentId].getComponent<CrowdAgent>().setGoal(goal);
}

void CrowdSimulation::setAgentMaxSpeed(size_t agentId, float speed)
{
  _simulator->setAgentMaxSpeed(agentId, speed);
}

void CrowdSimulation::addObstacleByBoundingBox(AbstractMesh* mesh,
                                               const Vector3& position,
                                               bool isVisible)
{
  mesh->setPosition(position);
  mesh->isVisible = isVisible;

  mesh->getBoundingInfo()->update(*mesh->getWorldMatrix());
  const auto bbox = mesh->getBoundingInfo()->boundingBox;
  const auto min  = bbox.minimumWorld;
  const auto max  = bbox.maximumWorld;

  // Add (polygonal) obstacle, specifying the vertices in counterclockwise
  // order.
  const vector_t<RVO2::Vector2> obstacle{
    RVO2::Vector2(max.x, max.z), //
    RVO2::Vector2(min.x, max.z), //
    RVO2::Vector2(min.x, min.z), //
    RVO2::Vector2(max.x, min.z)  //
  };

  _simulator->addObstacle(obstacle);
}

void CrowdSimulation::processObstacles()
{
  _simulator->processObstacles();
}

void CrowdSimulation::addWayPoint(const BABYLON::Vector2& waypoint)
{
  RoadmapVertex v;
  v.position = RVO2::Vector2(waypoint.x, waypoint.y);
  _roadmap.emplace_back(v);
}

void CrowdSimulation::computeRoadMap()
{
  /* Connect the roadmap vertices by edges if mutually visible. */
  for (size_t i = 0; i < _roadmap.size(); ++i) {
    for (size_t j = 0; j < _roadmap.size(); ++j) {
      if (_simulator->queryVisibility(_roadmap[i].position,
                                      _roadmap[j].position,
                                      _simulator->getAgentRadius(0))) {
        _roadmap[i].neighbors.push_back(j);
      }
    }

    // Initialize the distance to each of the four goal vertices at infinity
    // (9e9f).
    _roadmap[i].distToGoal.resize(4, 9e9f);
  }

  /*
   * Compute the distance to each of the four goals (the first four vertices)
   * for all vertices using Dijkstra's algorithm.
   */
  for (unsigned int i = 0; i < 4; ++i) {
    std::multimap<float, unsigned int> Q;
    std::vector<std::multimap<float, unsigned int>::iterator> posInQ(
      _roadmap.size(), Q.end());

    _roadmap[i].distToGoal[i] = 0.0f;
    posInQ[i]                 = Q.insert(std::make_pair(0.0f, i));

    while (!Q.empty()) {
      const auto u = Q.begin()->second;
      Q.erase(Q.begin());
      posInQ[u] = Q.end();

      for (size_t j = 0; j < _roadmap[u].neighbors.size(); ++j) {
        const auto v = _roadmap[u].neighbors[j];
        const float dist_uv
          = RVO2::abs(_roadmap[v].position - _roadmap[u].position);

        if (_roadmap[v].distToGoal[i] > _roadmap[u].distToGoal[i] + dist_uv) {
          _roadmap[v].distToGoal[i] = _roadmap[u].distToGoal[i] + dist_uv;

          if (posInQ[v] == Q.end()) {
            posInQ[v] = Q.insert(std::make_pair(_roadmap[v].distToGoal[i], v));
          }
          else {
            Q.erase(posInQ[v]);
            posInQ[v] = Q.insert(std::make_pair(_roadmap[v].distToGoal[i], v));
          }
        }
      }
    }
  }
}

bool CrowdSimulation::hasRoadMap() const
{
  return !_roadmap.empty();
}

const std::vector<RoadmapVertex>& CrowdSimulation::roadmap() const
{
  return _roadmap;
}

bool CrowdSimulation::isRunning() const
{
  // Check if all agents reached their goal
  long numEntitiesReachedGoal = std::count_if(
    _agents.begin(), _agents.end(), [](const ECS::Entity& entity) {
      return entity.getComponent<CrowdAgent>().reachedGoal();
    });

  return numEntitiesReachedGoal == static_cast<long>(_agents.size());
}

void CrowdSimulation::update()
{
  _world.refresh();
  // if (isRunning()) {
  _crowdCollisionAvoidanceSystem.update();
  _crowdMeshUpdaterSystem.update();
  //}
}

} // end of namespace Extensions
} // end of namespace BABYLON
