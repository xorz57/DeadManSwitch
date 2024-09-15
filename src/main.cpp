#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include <iostream>
#include <cstdlib>

static event *timer_event = nullptr;

void reset_timer(event_base *base);

void timeout_cb(evutil_socket_t fd, short events, void *arg) {
    std::cerr << "Dead man's switch triggered!\n";
    exit(EXIT_SUCCESS);
}

void http_reset_handler(evhttp_request *req, void *arg) {
    auto *base = static_cast<struct event_base *>(arg);

    std::cout << "Resetting!\n";
    reset_timer(base);

    evbuffer *response_buffer = evbuffer_new();
    evbuffer_add_printf(response_buffer, "Resetting!\n");
    evhttp_send_reply(req, HTTP_OK, "OK", response_buffer);
    evbuffer_free(response_buffer);
}

void http_shutdown_handler(evhttp_request *req, void *arg) {
    auto *base = static_cast<struct event_base *>(arg);

    std::cout << "Shutting down!\n";

    evhttp_send_reply(req, HTTP_OK, "Shutting down!", nullptr);

    event_base_loopbreak(base);
}

void reset_timer(event_base *base) {
    if (timer_event != nullptr) {
        evtimer_del(timer_event);
    }

    constexpr timeval timeout = {30, 0};
    timer_event = evtimer_new(base, timeout_cb, nullptr);
    evtimer_add(timer_event, &timeout);
}

int main() {
#ifdef _WIN32
    WSADATA wsa_data;
    if (const int wsa_error = WSAStartup(MAKEWORD(2, 2), &wsa_data); wsa_error != 0) {
        return EXIT_FAILURE;
    }
#endif

    event_base *base = event_base_new();
    if (!base) {
#ifdef _WIN32
        WSACleanup();
#endif
        return EXIT_FAILURE;
    }

    evhttp *http_server = evhttp_new(base);
    if (!http_server) {
        event_base_free(base);
#ifdef _WIN32
        WSACleanup();
#endif
        return EXIT_FAILURE;
    }

    if (evhttp_bind_socket(http_server, "0.0.0.0", 8080) != 0) {
        evhttp_free(http_server);
        event_base_free(base);
#ifdef _WIN32
        WSACleanup();
#endif
        return EXIT_FAILURE;
    }

    evhttp_set_cb(http_server, "/reset", http_reset_handler, base);
    evhttp_set_cb(http_server, "/shutdown", http_shutdown_handler, base);

    std::cout << "Resetting!\n";
    reset_timer(base);

    event_base_dispatch(base);

    evhttp_free(http_server);
    event_base_free(base);
#ifdef _WIN32
    WSACleanup();
#endif

    return EXIT_SUCCESS;
}
