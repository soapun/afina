#ifndef AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H

#include <afina/Storage.h>
#include <afina/execute/Command.h>
#include <cstring>
#include <protocol/Parser.h>
#include <spdlog/logger.h>
#include <sys/epoll.h>

namespace Afina {
namespace Network {
namespace STnonblock {

class Connection {
public:
    Connection(int s, std::shared_ptr<Afina::Storage> ps, std::shared_ptr<spdlog::logger> lg)
        : _socket(s), pStorage(ps), _logger(lg) {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
    }

    inline bool isAlive() const { return is_alive; }

    void Start();

protected:
    void OnError();
    void OnClose();
    void DoRead();
    void DoWrite();

private:
    friend class ServerImpl;

    int _socket;
    struct epoll_event _event;

    bool is_alive = true;
    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<Afina::Storage> pStorage;

    int cur_bytes;
    int head_written;
    std::vector<std::string> output;
    Protocol::Parser parser;
    std::size_t arg_remains;
    std::string argument_for_command;
    std::unique_ptr<Execute::Command> command_to_execute;
};

} // namespace STnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
