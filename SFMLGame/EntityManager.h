#pragma once

#include "Entity.h"
#include <vector>
#include <map>

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string, EntityVec> EntityMap;

class EntityManager
{
	EntityVec m_entities;
	EntityVec m_entitiesToAdd;
	EntityMap m_entityMap;
	size_t m_totalEntities = 0;

public:
	EntityManager();

	void update();
	EntityVec& getEntities();
	EntityVec& getEntities(const std::string& tag);
	std::shared_ptr<Entity> addEntity(const std::string& tag);
	void removeDeadEntities(EntityVec& entityVec);
};