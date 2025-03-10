#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <iostream>
#include <mutex>
#include <map>
#include <unordered_map>
#include <atomic>

namespace debug {
    template <typename T, typename = void>
    struct is_container : std::false_type
    {
    };

    template <typename T>
    struct is_container<T,
        std::void_t<decltype(std::begin(std::declval<T>())),
        decltype(std::end(std::declval<T>()))>> : std::true_type
    {
    };

    template <typename T>
    constexpr bool is_container_v = is_container<T>::value;

    template <typename T>
    struct is_map : std::false_type
    {
    };

    template <typename Key, typename Value>
    struct is_map<std::map<Key, Value>> : std::true_type
    {
    };

    template <typename T>
    constexpr bool is_map_v = is_map<T>::value;

    template <typename T>
    struct is_unordered_map : std::false_type
    {
    };

    template <typename Key, typename Value, typename Hash, typename KeyEqual,
        typename Allocator>
    struct is_unordered_map<
        std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>>
        : std::true_type
    {
    };

    template <typename T>
    constexpr bool is_unordered_map_v = is_unordered_map<T>::value;

    template <typename T>
    struct is_string : std::false_type
    {
    };

    template <>
    struct is_string<std::basic_string<char>> : std::true_type
    {
    };

    template <>
    struct is_string<const char*> : std::true_type
    {
    };

    template <typename T>
    constexpr bool is_string_v = is_string<T>::value;

    template <typename Container>
    std::enable_if_t<is_container_v<Container> && !is_map_v<Container>
        && !is_unordered_map_v<Container>,
        void>
        print_container(const Container& container);

    template <typename Map>
    std::enable_if_t<is_map_v<Map> || is_unordered_map_v<Map>, void>
        print_container(const Map& map);

    extern std::mutex log_mutex;
    template <typename ParamType>
    void _log(const ParamType& param);
    template <typename ParamType, typename... Args>
    void _log(const ParamType& param, const Args&... args);

    /////////////////////////////////////////////////////////////////////////////////////////////

    template <typename... Args> void log(const Args&... args);

    template <typename Container>
    std::enable_if_t < debug::is_container_v<Container>
        && !debug::is_map_v<Container>
        && !debug::is_unordered_map_v<Container>
        , void >
        print_container(const Container& container)
    {
        std::cerr << "[";
        for (auto it = std::begin(container); it != std::end(container); ++it)
        {
            _log(*it);
            if (std::next(it) != std::end(container)) {
                std::cerr << ", ";
            }
        }
        std::cerr << "]";
    }

    template <typename Map>
    std::enable_if_t < debug::is_map_v<Map> || debug::is_unordered_map_v<Map>, void >
        print_container(const Map& map)
    {
        std::cerr << "{";
        for (auto it = std::begin(map); it != std::end(map); ++it)
        {
            _log(it->first);
            std::cerr << ": ";
            _log(it->second);
            if (std::next(it) != std::end(map)) {
                std::cerr << ", ";
            }
        }
        std::cerr << "}";
    }

    template <typename ParamType> void _log(const ParamType& param)
    {
        if constexpr (debug::is_string_v<ParamType>) {
            // Handle std::string and const char*
            std::cerr << param;
        }
        else if constexpr (debug::is_container_v<ParamType>) {
            // Handle containers
            debug::print_container(param);
        }
        else {
            // Handle other types
            std::cerr << param;
        }
    }

    template <typename ParamType, typename... Args>
    void _log(const ParamType& param, const Args &...args)
    {
        _log(param);
        (_log(args), ...);
    }

    template <typename... Args> void log(const Args &...args)
    {
        setvbuf(stderr, nullptr, _IONBF, 0);
        std::lock_guard<std::mutex> lock(log_mutex);
        debug::_log(args...);
        std::cerr << std::flush;
        fflush(stderr);
    }
}

#endif // _LOG_HPP_
