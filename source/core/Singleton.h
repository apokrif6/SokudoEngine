#pragma once

namespace Core
{
template <typename T> class Singleton
{
public:
    static T& getInstance()
    {
        static T instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;

    Singleton& operator=(const Singleton&) = delete;

    Singleton(Singleton&&) = delete;

    Singleton& operator=(Singleton&&) = delete;

protected:
    Singleton() = default;

    virtual ~Singleton() = default;
};
} // namespace Core