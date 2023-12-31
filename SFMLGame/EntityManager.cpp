#include "EntityManager.h"

EntityManager::EntityManager() {}

void EntityManager::update()
{
	for (auto& entity : m_entitiesToAdd)
	{
		m_entities.push_back(entity);
		m_entityMap[entity->tag()].push_back(entity);
	}

	m_entitiesToAdd.clear();

	removeDeadEntities(m_entities);

	for (auto& tag_entityVec : m_entityMap)
	{
		removeDeadEntities(tag_entityVec.second);
	}
}

EntityVec& EntityManager::getEntities()
{
	return m_entities;
}

EntityVec& EntityManager::getEntities(const std::string& tag)
{
	return m_entityMap[tag];
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
	auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

	m_entitiesToAdd.push_back(entity);

	return entity;
}

void EntityManager::removeDeadEntities(EntityVec& entityVec)
{
	entityVec.erase(std::remove_if(entityVec.begin(), entityVec.end(), [](const std::shared_ptr<Entity>& entity) {
		return !entity->isActive(); 
		}), entityVec.end());
}