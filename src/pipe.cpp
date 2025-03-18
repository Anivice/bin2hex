#include "pipe.hpp"
#include "log.hpp"

#include <stdexcept>

constexpr int max_line_char_num = 64;

namespace bin2hex {
    char hex_table [] = {
        '0', 0x00,
        '1', 0x01,
        '2', 0x02,
        '3', 0x03,
        '4', 0x04,
        '5', 0x05,
        '6', 0x06,
        '7', 0x07,
        '8', 0x08,
        '9', 0x09,
#ifndef LOWER_CASE_MAP
        'A', 0x0A,
        'B', 0x0B,
        'C', 0x0C,
        'D', 0x0D,
        'E', 0x0E,
        'F', 0x0F,
#else
        'a', 0x0A,
        'b', 0x0B,
        'c', 0x0C,
        'd', 0x0D,
        'e', 0x0E,
        'f', 0x0F,
#endif // LOWER_CASE_MAP
    };

    bool is_valid(const char hex) {
#ifndef LOWER_CASE_MAP
        return ((hex >= '0' && hex <= '9') || (hex >= 'A' && hex <= 'F'));
#else
        return ((hex >= '0' && hex <= '9') || (hex >= 'a' && hex <= 'f'));
#endif // LOWER_CASE_MAP
    }

    char c_hex2bin(const char hex_a, const char hex_b)
    {
        char result = 0;

        if (!is_valid(hex_a) || !is_valid(hex_b)) {
            throw std::invalid_argument("Invalid hex string pair `" + std::to_string((int)hex_a) + "d+" + std::to_string((int)hex_b) + "d`");
        }

        auto find_in_table = [](const char hex)->char
        {
            for (size_t i = 0; i < sizeof(hex_table); i += 2) {
                if (hex_table[i] == hex) {
                    return hex_table[i + 1];
                }
            }

            throw std::invalid_argument("Invalid hex code");
        };

        result = find_in_table(hex_b);
        result |= static_cast<char>(find_in_table(hex_a) << 4);

        return result;
    }

    void c_bin2hex(const char bin, char hex[2])
    {
        auto find_in_table = [](const char p_hex) -> char {
            for (size_t i = 0; i < sizeof(hex_table); i += 2) {
                if (hex_table[i + 1] == p_hex) {
                    return hex_table[i];
                }
            }

            throw std::invalid_argument("Invalid binary code");
        };

        const char bin_a = static_cast<char>(bin >> 4) & 0x0F;
        const char bin_b = bin & 0x0F;

        hex[0] = find_in_table(bin_a);
        hex[1] = find_in_table(bin_b);
    }

    std::string bin2hex(const std::vector < char > & vec)
    {
        int cur_line_char_num = 0;

        std::string result;
        char buffer [3] { };
        for (const auto & bin : vec) {
            c_bin2hex(bin, buffer);
            result += buffer;
            cur_line_char_num += 2;

            if (cur_line_char_num == max_line_char_num) {
                result += "\n";
                cur_line_char_num = 0;
            }
        }

        return result;
    }

    std::vector < char > hex2bin(const std::string & hex) {
        try {
            std::vector < char > ret;

            for (std::size_t i = 0; i < hex.length(); i += 2) 
            {
                char code_1 = hex[i], code_2 = hex[i + 1];
                if (code_1 == '\n') 
                {
                    if ((i + 1) == hex.length()) {
                        break;
                    }

                    i += 1;
                    code_1 = hex[i];
                    code_2 = hex[i + 1];
                }

                if (code_2 == '\n') {
                    i += 1;

                    if ((i + 1) >= hex.length()) {
                        throw std::invalid_argument("Invalid hex alignment");
                    }

                    code_2 = hex[i + 1];
                }

                ret.emplace_back(c_hex2bin(code_1, code_2));
            }

            return ret;
        } catch (std::out_of_range &) {
            throw std::invalid_argument("Invalid hex alignment");
        }
    }
}
