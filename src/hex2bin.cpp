#include "log.hpp"
#include "pipe.hpp"

#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>

#define SZ_256KB (256 * 1024)

int main(int argc, char *argv[])
{
    try {
        int fd_src = -1, fd_dest = -1;
        std::vector < char > buffer_256K {};
        long long current_page_size = 0;

        if (argc < 3) {
            debug::log("Usage: ", *argv, " <hex file> <output file>\n");
            return EXIT_FAILURE;
        }

        if (strcmp(argv[1], "-") == 0) {
            fd_src = STDIN_FILENO;
        }

        if (strcmp(argv[2], "-") == 0) {
            fd_dest = STDOUT_FILENO;
        }

        if (fd_src == -1) {
            fd_src = open(argv[1], O_RDONLY);
        }

        if (fd_dest == -1) {
            fd_dest = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        }

        if (fd_src == -1 || fd_dest == -1) {
            debug::log("Error opening file: ", strerror(errno), "\n");
            return EXIT_FAILURE;
        }

        bool last_is_nl = false;

        do {
            buffer_256K.resize(SZ_256KB);
            current_page_size = read(fd_src, buffer_256K.data(), SZ_256KB);
            if (current_page_size == -1) {
                debug::log("Error reading file: ", strerror(errno), "\n");
                return EXIT_FAILURE;
            }

            if (current_page_size != SZ_256KB) {
                buffer_256K.resize(current_page_size);
            }

            const auto bin_data = bin2hex::hex2bin({buffer_256K.begin(), buffer_256K.end()});
            if (const auto write_len = write(fd_dest, bin_data.data(), bin_data.size());
                write_len != static_cast<ssize_t>(bin_data.size()) )
            {
                debug::log("Error writing file: ", strerror(errno), "\n");
                exit(EXIT_FAILURE);
            }

            if (!bin_data.empty()) {
                last_is_nl = bin_data.back() == '\n';
            }
        } while (current_page_size > 0);

        if (fd_dest == STDOUT_FILENO && !last_is_nl) {
            if (write(fd_dest, "\n", 1) != 1) {
                debug::log("Error writing file: ", strerror(errno), "\n");
                exit(EXIT_FAILURE);
            }
        }

        close(fd_src);
        close(fd_dest);

        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        debug::log("Exception occured: ", e.what(), "\n");
        return EXIT_FAILURE;
    } catch (...) {
        debug::log("Unknown exception occured!", "\n");
        return EXIT_FAILURE;
    }
}
