#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <iostream>
#include <mutex>
#include <map>
#include <unordered_map>
#include <atomic>

#define construct_simple_type_compare(type)                             \
    template <typename T>                                               \
    struct is_##type : std::false_type {};                              \
    template <>                                                         \
    struct is_##type<type> : std::true_type { };                        \
    template <typename T>                                               \
    constexpr bool is_##type##_v = is_##type<T>::value;

#ifndef __LOG_TO_STDOUT__
# define LOG_DEV std::cerr
# define LOG_DEV_FILE (stderr)
#else
# define LOG_DEV std::cout
# define LOG_DEV_FILE (stdout)
#endif

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

    construct_simple_type_compare(bool);

    inline class lower_case_bool_t {} lower_case_bool;
    construct_simple_type_compare(lower_case_bool_t);

    inline class upper_case_bool_t {} upper_case_bool;
    construct_simple_type_compare(upper_case_bool_t);

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
        LOG_DEV << "[";
        for (auto it = std::begin(container); it != std::end(container); ++it)
        {
            _log(*it);
            if (std::next(it) != std::end(container)) {
                LOG_DEV << ", ";
            }
        }
        LOG_DEV << "]";
    }

    template <typename Map>
    std::enable_if_t < debug::is_map_v<Map> || debug::is_unordered_map_v<Map>, void >
        print_container(const Map& map)
    {
        LOG_DEV << "{";
        for (auto it = std::begin(map); it != std::end(map); ++it)
        {
            _log(it->first);
            LOG_DEV << ": ";
            _log(it->second);
            if (std::next(it) != std::end(map)) {
                LOG_DEV << ", ";
            }
        }
        LOG_DEV << "}";
    }

    extern std::string str_true;
    extern std::string str_false;

    template <typename ParamType> void _log(const ParamType& param)
    {
        if constexpr (debug::is_string_v<ParamType>) {
            // Handle std::string and const char*
            LOG_DEV << param;
        }
        else if constexpr (debug::is_container_v<ParamType>) {
            // Handle containers
            debug::print_container(param);
        }
        else if constexpr (debug::is_bool_v<ParamType>) {
            // Handle bool
            LOG_DEV << (param ? str_true : str_false);
        }
        else if constexpr (debug::is_lower_case_bool_t_v<ParamType>) {
            // change bool output
            str_true = "true";
            str_false = "false";
        }
        else if constexpr (debug::is_upper_case_bool_t_v<ParamType>) {
            // change bool output
            str_true = "True";
            str_false = "False";
        }
        else {
            // Handle other types
            LOG_DEV << param;
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
        setvbuf(LOG_DEV_FILE, nullptr, _IONBF, 0);
        std::lock_guard<std::mutex> lock(log_mutex);
        debug::_log(args...);
        LOG_DEV << std::flush;
        fflush(LOG_DEV_FILE);
    }
}

#endif // _LOG_HPP_
