#pragma once

namespace safe
{
    template <typename T>
    T & notnull(T * value)
    {
        if (value == nullptr)
        {
            throw std::invalid_argument("Value is null");
        }
        return *value;
    }
}
