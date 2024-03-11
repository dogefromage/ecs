#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <memory>
#include <queue>
#include <set>
#include <unordered_map>

// follows pretty closely https://austinmorlan.com/posts/entity_component_system/

using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 5000;

using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;

using Signature = std::bitset<MAX_COMPONENTS>;

class EntityManager {
   public:
    EntityManager() {
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
            unusedEntities.push(entity);
        }
    }

    Entity createEntity() {
        assert(livingEntityCount < MAX_ENTITIES && "Too many entities in existence.");

        Entity id = unusedEntities.front();
        unusedEntities.pop();
        ++livingEntityCount;

        return id;
    }

    void destroyEntity(Entity entity) {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        signatures[entity].reset();
        unusedEntities.push(entity);
        --livingEntityCount;
    }

    void setSignature(Entity entity, Signature signature) {
        assert(entity < MAX_ENTITIES && "Entity out of range.");
        signatures[entity] = signature;
    }

    Signature getSignature(Entity entity) {
        assert(entity < MAX_ENTITIES && "Entity out of range.");
        return signatures[entity];
    }

   private:
    std::queue<Entity> unusedEntities{};
    std::array<Signature, MAX_ENTITIES> signatures{};
    uint32_t livingEntityCount{};
};

class IComponentArray {
   public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
   public:
    void insertData(Entity entity, T component) {
        assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Component added to the same entity more than once.");
        // Put new entry at the end and update the maps
        size_t newIndex = size;
        entityToIndexMap[entity] = newIndex;
        indexToEntityMap[newIndex] = entity;
        componentArray[newIndex] = component;
        ++size;
    }

    void removeData(Entity entity) {
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Removing a non-existent component.");

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

    T& getData(Entity entity) {
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Retrieving a non-existent component.");

        return componentArray[entityToIndexMap[entity]];
    }

    void entityDestroyed(Entity entity) override {
        if (entityToIndexMap.find(entity) != entityToIndexMap.end()) {
            removeData(entity);
        }
    }

   private:
    std::array<T, MAX_ENTITIES> componentArray;
    std::unordered_map<Entity, size_t> entityToIndexMap;
    std::unordered_map<size_t, Entity> indexToEntityMap;
    size_t size;
};

class ComponentManager {
   public:
    template <typename T>
    void registerComponent() {
        const char* typeName = typeid(T).name();
        assert(componentTypes.find(typeName) == componentTypes.end() && "Registering a component type more than once.");
        componentTypes.insert({typeName, nextComponentType});
        componentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});
        nextComponentType++;
    }

    template <typename T>
    ComponentType getComponentType() {
        const char* typeName = typeid(T).name();
        assert(componentTypes.find(typeName) != componentTypes.end() && "Component not registered before use.");
        return componentTypes[typeName];
    }

    template <typename T>
    void addComponent(Entity entity, T component) {
        getComponentArray<T>()->insertData(entity, component);
    }

    template <typename T>
    void removeComponent(Entity entity) {
        getComponentArray<T>()->removeData(entity);
    }

    template <typename T>
    T& getComponent(Entity entity) {
        return getComponentArray<T>()->getData(entity);
    }

    void entityDestroyed(Entity entity) {
        for (auto const& pair : componentArrays) {
            auto const& component = pair.second;
            component->entityDestroyed(entity);
        }
    }

   private:
    std::unordered_map<const char*, ComponentType> componentTypes{};
    std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays{};
    ComponentType nextComponentType{};

    template <typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() {
        const char* typeName = typeid(T).name();
        assert(componentTypes.find(typeName) != componentTypes.end() && "Component not registered before use.");
        return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
    }
};

class System {
   public:
    std::set<Entity> entities;
};

class SystemManager {
   public:
    template <typename T>
    std::shared_ptr<T> registerSystem() {
        const char* typeName = typeid(T).name();
        assert(systems.find(typeName) == systems.end() && "Registering a system more than once.");
        auto system = std::make_shared<T>();
        systems.insert({typeName, system});
        return system;
    }

    template <typename T>
    void setSignature(Signature signature) {
        const char* typeName = typeid(T).name();
        assert(systems.find(typeName) != systems.end() && "System used before registered.");
        signatures.insert({typeName, signature});
    }

    void entityDestroyed(Entity entity) {
        for (auto const& pair : systems) {
            auto const& system = pair.second;
            system->entities.erase(entity);
        }
    }

    void entitySignatureChanged(Entity entity, Signature entitySignature) {
        for (auto const& pair : systems) {
            auto const& type = pair.first;
            auto const& system = pair.second;
            auto const& systemSignature = signatures[type];

            // Entity signature matches system signature - insert into the set
            if ((entitySignature & systemSignature) == systemSignature) {
                system->entities.insert(entity);
            }
            // Entity signature does not match system signature - erase from the set
            else {
                system->entities.erase(entity);
            }
        }
    }

   private:
    std::unordered_map<const char*, Signature> signatures{};
    std::unordered_map<const char*, std::shared_ptr<System>> systems{};
};

class Coordinator {
   public:
    void init() {
        // Create pointers to each manager
        componentManager = std::make_unique<ComponentManager>();
        entityManager = std::make_unique<EntityManager>();
        systemManager = std::make_unique<SystemManager>();
    }

    // Entity methods
    Entity createEntity() {
        return entityManager->createEntity();
    }

    void destroyEntity(Entity entity) {
        entityManager->destroyEntity(entity);
        componentManager->entityDestroyed(entity);
        systemManager->entityDestroyed(entity);
    }

    // Component methods
    template <typename T>
    void registerComponent() {
        componentManager->registerComponent<T>();
    }

    template <typename T>
    void addComponent(Entity entity, T component) {
        componentManager->addComponent<T>(entity, component);

        auto signature = entityManager->getSignature(entity);
        signature.set(componentManager->getComponentType<T>(), true);
        entityManager->setSignature(entity, signature);

        systemManager->entitySignatureChanged(entity, signature);
    }

    template <typename T>
    void removeComponent(Entity entity) {
        componentManager->removeComponent<T>(entity);

        auto signature = entityManager->getSignature(entity);
        signature.set(componentManager->getComponentType<T>(), false);
        entityManager->setSignature(entity, signature);

        systemManager->entitySignatureChanged(entity, signature);
    }

    template <typename T>
    T& getComponent(Entity entity) {
        return componentManager->getComponent<T>(entity);
    }

    template <typename T>
    ComponentType getComponentType() {
        return componentManager->getComponentType<T>();
    }

    // System methods
    template <typename T>
    std::shared_ptr<T> registerSystem() {
        return systemManager->registerSystem<T>();
    }

    template <typename T>
    void setSystemSignature(Signature signature) {
        systemManager->setSignature<T>(signature);
    }

   private:
    std::unique_ptr<ComponentManager> componentManager;
    std::unique_ptr<EntityManager> entityManager;
    std::unique_ptr<SystemManager> systemManager;
};

extern Coordinator gCoordinator;
