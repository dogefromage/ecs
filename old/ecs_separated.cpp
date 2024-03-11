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

    size_t indexOfRemoved = entityToIndexMap[entity];
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
    assert(componentTypes.find(typeName) == componentTypes.end() && "Registering component type more than once.");
    componentTypes.insert({typeName, nextComponentType});
    componentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});
    nextComponentType++;
}

template <typename T>
ComponentType ComponentManager::getComponentType() {
    const char* typeName = typeid(T).name();
    assert(componentTypes.find(typeName) != componentTypes.end() && "Component not registered before use.");
    return componentTypes[typeName];
}

template <typename T>
void ComponentManager::addComponent(Entity entity, T component) {
    getComponentArray<T>()->insertData(entity, component);
}

template <typename T>
void ComponentManager::removeComponent(Entity entity) {
    getComponentArray<T>()->removeData(entity);
}

template <typename T>
T& ComponentManager::getComponent(Entity entity) {
    return getComponentArray<T>()->getData(entity);
}

void ComponentManager::entityDestroyed(Entity entity) {
    for (auto const& pair : componentArrays) {
        auto const& component = pair.second;
        component->entityDestroyed(entity);
    }
}

template <typename T>
std::shared_ptr<ComponentArray<T>> ComponentManager::getComponentArray() {
    const char* typeName = typeid(T).name();
    assert(componentTypes.find(typeName) != componentTypes.end() && "Component not registered before use.");
    return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
}

template <typename T>
std::shared_ptr<T> SystemManager::registerSystem() {
    const char* typeName = typeid(T).name();
    assert(systems.find(typeName) == systems.end() && "Registering system more than once.");
    auto system = std::make_shared<T>();
    systems.insert({typeName, system});
    return system;
}

template <typename T>
void SystemManager::setSignature(Signature signature) {
    const char* typeName = typeid(T).name();
    assert(systems.find(typeName) != systems.end() && "System used before registered.");
    signatures.insert({typeName, signature});
}

void SystemManager::entityDestroyed(Entity entity) {
    for (auto const& pair : systems) {
        auto const& system = pair.second;
        system->entities.erase(entity);
    }
}

void SystemManager::entitySignatureChanged(Entity entity, Signature entitySignature) {
    for (auto const& pair : systems) {
        auto const& type = pair.first;
        auto const& system = pair.second;
        auto const& systemSignature = signatures[type];

        // Entity signature matches system signature - insert into set
        if ((entitySignature & systemSignature) == systemSignature) {
            system->entities.insert(entity);
        }
        // Entity signature does not match system signature - erase from set
        else {
            system->entities.erase(entity);
        }
    }
}

void Coordinator::init() {
    // Create pointers to each manager
    componentManager = std::make_unique<ComponentManager>();
    entityManager = std::make_unique<EntityManager>();
    systemManager = std::make_unique<SystemManager>();
}

// Entity methods
Entity Coordinator::createEntity() {
    return entityManager->createEntity();
}

void Coordinator::destroyEntity(Entity entity) {
    entityManager->destroyEntity(entity);
    componentManager->entityDestroyed(entity);
    systemManager->entityDestroyed(entity);
}

// Component methods
template <typename T>
void Coordinator::registerComponent() {
    componentManager->registerComponent<T>();
}

template <typename T>
void Coordinator::addComponent(Entity entity, T component) {
    componentManager->addComponent<T>(entity, component);

    auto signature = entityManager->getSignature(entity);
    signature.set(componentManager->getComponentType<T>(), true);
    entityManager->setSignature(entity, signature);

    systemManager->entitySignatureChanged(entity, signature);
}

template <typename T>
void Coordinator::removeComponent(Entity entity) {
    componentManager->removeComponent<T>(entity);

    auto signature = entityManager->getSignature(entity);
    signature.set(componentManager->getComponentType<T>(), false);
    entityManager->setSignature(entity, signature);

    systemManager->entitySignatureChanged(entity, signature);
}

template <typename T>
T& Coordinator::getComponent(Entity entity) {
    return componentManager->getComponent<T>(entity);
}

template <typename T>
ComponentType Coordinator::getComponentType() {
    return componentManager->getComponentType<T>();
}

// System methods
template <typename T>
std::shared_ptr<T> Coordinator::registerSystem() {
    return systemManager->registerSystem<T>();
}

template <typename T>
void Coordinator::setSystemSignature(Signature signature) {
    systemManager->setSignature<T>(signature);
}

Coordinator gCoordinator;