#pragma once

template<typename T>
struct Hash_fn
{
    std::size_t operator()(const T& s) const
    {
        std::hash<uint32> hasher;
        const uint32* data = (const uint32*)&s;
        size_t              seed = 0;
        for (size_t i = 0; i < sizeof(T) / sizeof(uint32); i++)
        {
            // https://www.boost.org/doc/libs/1_35_0/doc/html/boost/hash_combine_id241013.html
            seed ^= hasher(data[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};
