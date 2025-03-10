#ifndef PIPE_H
#define PIPE_H

#include <vector>
#include <string>

namespace bin2hex {
    char c_hex2bin(char hex_a, char hex_b);
    void c_bin2hex(char bin, char hex[2]);

    std::string bin2hex(const std::vector < char > &);
    inline std::string bin2hex(const std::string & str) {
        const std::vector < char > vec(str.begin(), str.end());
        return bin2hex::bin2hex(vec);
    }

    std::vector < char > hex2bin(const std::string & hex);
    inline std::string hex2bin_s(const std::string & hex) {
        auto bin = hex2bin(hex);
        return { bin.begin(), bin.end() };
    }
} // bin2hex

#endif //PIPE_H
