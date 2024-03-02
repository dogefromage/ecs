#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <shared_ptr.h>

// follows pretty closely https://austinmorlan.com/posts/entity_component_system/

using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 5000;

using Signature = std::bitset<MAX_COMPONENTS>;

using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;

class EntityManager {
public:
    EntityManager();

    Entity createEntity();
    void destroyEntity(Entity entity);
    void setSignature(Entity entity, Signature signature);
    Signature getSignature(Entity entity);

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
class ComponentArray : IComponentArray {
   public:
    void insertData(Entity entity, T component);
    void removeData(Entity entity);
    T& getData(Entity entity);
    void entityDestroyed(Entity entity) override;

   private:
    std::array<T, MAX_ENTITIES> componentArray;
    std::unordered_map<Entity, size_t> entityToIndexMap;
    std::unordered_map<size_t, Entity> indexToEntityMap;
    size_t size;
};

class ComponentManager {
    public:
    template<typename T>
    void registerComponent();

    template<typename T>
    ComponentType getComponentType();

    template<typename T>
    void addComponent(Entity entity, T component);

    template<typename T>
    void removeComponent(Entity entity);

    template <typename T>
    T& getComponent(Entity entity);

    void entityDestroyed(Entity entity);

    private:
    std::unordered_map<const char*, ComponentType> componentTypes{};
    std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays{};
    ComponentType nextComponentType{};

    template <typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray();
};

