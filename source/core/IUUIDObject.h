#pragma once

#include "uuid.h"

namespace Core
{
namespace UUID
{
inline uuids::uuid generateUUID()
{
    static std::random_device randomDevice;
    static std::array<int, std::mt19937::state_size> seed_data;
    std::ranges::generate(seed_data, std::ref(randomDevice));
    std::seed_seq seedSequence(std::begin(seed_data), std::end(seed_data));
    static std::mt19937 generator(seedSequence);

    static uuids::uuid_random_generator randomGenerator(generator);

    return randomGenerator();
}
} // namespace UUID
class IUUIDObject
{
public:
    virtual ~IUUIDObject() = default;

    [[nodiscard]] virtual const uuids::uuid& getUUID() const = 0;
};
} // namespace Core