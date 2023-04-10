#pragma once

#include "Entt/entt.hpp"
#include "Entity.h"

#include "Graphics/Renderer.h"

namespace Okay
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity createEntity();
		inline void destroyEntity(const Entity& entity);
		inline void destroyEntity(entt::entity entity);

		inline void setMainCamera(const Entity& entity);
		inline void setSkyLight(const Entity& entity);

		inline Entity getMainCamera();
		inline Entity getSkyLight();

		inline entt::registry& getRegistry();

		void start();
		void update();
		void end();

	private:
		entt::registry registry;

		Entity mainCamera;
		Entity skyLight;
	};

	inline void Scene::destroyEntity(const Entity& entity)	{ registry.destroy(entity); }
	inline void Scene::destroyEntity(entt::entity entity)	{ registry.destroy(entity); }

	inline void Scene::setMainCamera(const Entity& entity)	{ mainCamera = entity; }
	inline void Scene::setSkyLight(const Entity& entity)	{ skyLight = entity; }

	inline Entity Scene::getMainCamera()					{ return mainCamera; }
	inline Entity Scene::getSkyLight()						{ return skyLight; }

	inline entt::registry& Scene::getRegistry()				{ return registry; }
}