#pragma once

#include "Okay/Okay.h"

#include "Entt/entt.hpp"

namespace Okay
{
	class Entity
	{
	public:
		Entity()
			:entityId(entt::null), pReg(nullptr) 
		{ }

		Entity(entt::entity id, entt::registry* pReg)
			:entityId((entt::entity)id), pReg(pReg) 
		{ }
		

		template<typename T, typename... Args>
		inline T& addComponent(Args&&... args);

		template<typename... T>
		inline bool hasComponent() const;

		template<typename T>
		inline T& getComponent();

		template<typename T>
		inline const T& getComponent() const;
		
		template<typename T>
		inline T* tryGetComponent();

		template<typename T>
		inline const T* tryGetComponent() const;

		template<typename T>
		inline bool removeComponent();

		inline operator entt::entity() const	{ return entityId; }
		inline uint32_t getID() const			{ return (uint32_t)entityId; }

		inline explicit operator bool() const			{ return isValid(); }
		inline bool isValid() const				{ return pReg ? pReg->valid(entityId) : false; }

		inline bool operator== (const Okay::Entity& other) { return entityId == other.entityId; }

	private:
		entt::registry* pReg; // Hmmmmm
		entt::entity entityId;

		// Due to padding, another 4 bytes can fit here for free
		// What to add thooo :thonk:
	};


	template<typename T, typename... Args>
	inline T& Entity::addComponent(Args&&... args)
	{
		OKAY_ASSERT(pReg, "pReg was nullptr");
		return pReg->emplace<T>(entityId, std::forward<Args>(args)...);
	}

	template<typename... T>
	inline bool Entity::hasComponent() const
	{
		OKAY_ASSERT(pReg, "pReg was nullptr");
		return pReg->all_of<T...>(entityId);
	}

	template<typename T>
	inline T& Entity::getComponent()
	{
		OKAY_ASSERT(pReg, "pReg was nullptr");
		OKAY_ASSERT(hasComponent<T>(), "The entity doesn't have the given component");
		return pReg->get<T>(entityId);
	}

	template<typename T>
	inline const T& Entity::getComponent() const
	{
		OKAY_ASSERT(pReg, "pReg was nullptr");
		OKAY_ASSERT(hasComponent<T>(), "The entity doesn't have the given component");
		return pReg->get<T>(entityId);
	}

	template<typename T>
	inline T* Entity::tryGetComponent()
	{
		OKAY_ASSERT(pReg, "pReg was nullptr");
		return pReg->try_get<T>(entityId);
	}

	template<typename T>
	inline const T* Entity::tryGetComponent() const
	{
		OKAY_ASSERT(pReg, "pReg was nullptr");
		return pReg->try_get<T>(entityId);
	}

	template<typename T>
	inline bool Entity::removeComponent()
	{
		OKAY_ASSERT(pReg, "pReg was nullptr");
		return pReg->remove<T>(entityId);
	}
}