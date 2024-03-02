#include "ecs.h"

EntityManager::EntityManager() {
    for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
        unusedEntities.push(entity);
    }
}

Entity EntityManager::createEntity() {
    assert(livingEntityCount < MAX_ENTITIES && "Too many entities in existence.");

    Entity id = unusedEntities.front();
    unusedEntities.pop();
    ++livingEntityCount;

    return id;
}

void EntityManager::destroyEntity(Entity entity) {
    assert(entity < MAX_ENTITIES && "Entity out of range.");

    signatures[entity].reset();
    unusedEntities.push(entity);
    --livingEntityCount;
}

void EntityManager::setSignature(Entity entity, Signature signature) {
    assert(entity < MAX_ENTITIES && "Entity out of range.");
    signatures[entity] = signature;
}

Signature EntityManager::getSignature(Entity entity) {
    assert(entity < MAX_ENTITIES && "Entity out of range.");
    return signatures[entity];
}

template <typename T>
void ComponentArray<T>::insertData(Entity entity, T component) {
    assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Component added to same entity more than once.");
    // Put new entry at end and update the maps
    size_t newIndex = size;
    entityToIndexMap[entity] = newIndex;
    indexToEntityMap[newIndex] = entity;
    componentArray[newIndex] = component;
    ++size;
}

template <typename T>
void ComponentArray<T>::removeData(Entity entity) {
    assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Removing non-existent component.");

    size_t indexOfRemoved = entityToIndexMap.find[entity];
    size_t indexOfLast = size - 1;
    componentArray[indexOfRemoved] = componentArray[indexOfLast];

    Entity entityOfLast = indexToEntityMap[indexOfLast];
    entityToIndexMap[entityOfLast] = indexOfRemoved;
    indexToEntityMap[indexOfRemoved] = entityOfLast;

    indexToEntityMap.erase(indexOfLast);
    entityToIndexMap.erase(entity);
    size--;
}

template <typename T>
T& ComponentArray<T>::getData(Entity entity) {
    assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Retrieving non-existent component.");

    return componentArray[entityToIndexMap[entity]];
}

template <typename T>
void ComponentArray<T>::entityDestroyed(Entity entity) {
    if (entityToIndexMap.find(entity) != entityToIndexMap.end()) {
        removeData(entity);
    }
}

template <typename T>
void ComponentManager::registerComponent() {
    const char* typeName = typeid(T).name();
    assert(mComponentTypes.find(typeName) == mComponentTypes.end() && "Registering component type more than once.");
    componentTypes.insert({typeName, nextComponentType});
    componentArrays.insert({typename, std::make_shared<ComponentArray<T>>()});
    nextComponentType++;
}

template <typename T>
ComponentType ComponentManager::getComponentType() {
    return ComponentType();
}

template <typename T>
void ComponentManager::addComponent(Entity entity, T component) {
}

template <typename T>
void ComponentManager::removeComponent(Entity entity) {
}

template <typename T>
T& ComponentManager::getComponent(Entity entity) {
    // TODO: insert return statement here
}

void ComponentManager::entityDestroyed(Entity entity) {
}
