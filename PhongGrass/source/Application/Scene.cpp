#include "Scene.h"
#include "Entity.h"
#include "Components/Transform.h"
#include "Components/MeshComponent.h"
#include "Graphics/Renderer.h"

namespace Okay
{
    Scene::Scene()
    {
    }
    
    Scene::~Scene()
    {
    }
    
    Entity Scene::createEntity()
    {
        Entity entity(registry.create(), &registry);
        entity.addComponent<Okay::Transform>();
        
        return entity;
    }

    void Scene::start()
    {

    }
    
    void Scene::update()
    {

    }
    
    void Scene::end()
    {

    }
}
