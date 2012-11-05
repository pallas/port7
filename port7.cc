/*
 * port7 - a simple echo service
 * Author: Derrick Pallas
 */

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <stdexcept>

#include <arpa/inet.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>

#define TRY_PTR(f, ...) ({ \
    typeof (f(__VA_ARGS__)) _r = f(__VA_ARGS__); \
    if (!_r) throw std::runtime_error(#f); \
    _r; \
})

template <typename T, void (*F)(T*)>
struct cleanup {
    cleanup(T &t) : _(t) {}
    ~cleanup() { F(&_); }
private:
    T &_;
};

static void
on_event(struct bufferevent *bev, short events, void *) {
    if (events & BEV_EVENT_ERROR) {
        bufferevent_free(bev);
        return;
    }

    if (events & BEV_EVENT_EOF) {
        bufferevent_free(bev);
        return;
    }
}

static void
on_read(struct bufferevent *bev, void *) {
    struct evbuffer &i = *bufferevent_get_input(bev);
    struct evbuffer &o = *bufferevent_get_output(bev);
    evbuffer_add_buffer(&o, &i);
}

static void
on_error(struct evconnlistener *listener, void *) {
    struct event_base *base = evconnlistener_get_base(listener);
    throw std::runtime_error(__FUNCTION__);
}

static void
on_accept( struct evconnlistener *listener, evutil_socket_t fd
         , struct sockaddr *, int, void *ctx ) {
    //
    struct event_base &base = *evconnlistener_get_base(listener);
    struct bufferevent &bev = *TRY_PTR(bufferevent_socket_new,
        &base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(&bev, on_read, NULL, on_event, NULL);
    bufferevent_enable(&bev, EV_READ|EV_WRITE);
}

int
main(int argc, char *argv[]) try {
    assert((LIBEVENT_VERSION_NUMBER >> 16) == (event_get_version_number() >> 16));
#if LIBEVENT_VERSION_NUMBER >= 0x02010100
    atexit(libevent_global_shutdown);
# ifdef DEBUG
    event_enable_debug_logging(EVENT_DBG_ALL);
# endif// DEBUG
#endif// 2.1.1

    struct event_base &base = *TRY_PTR(event_base_new);
    cleanup<struct event_base, event_base_free> _base(base);

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof sin);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0);
    sin.sin_port = htons(7);

    const struct sockaddr * sa = reinterpret_cast<const struct sockaddr*>(&sin);
    int flags = LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE;
    struct evconnlistener &listener = *TRY_PTR(evconnlistener_new_bind,
        &base, on_accept, NULL, flags, -1, sa, sizeof sin);
    cleanup<struct evconnlistener, evconnlistener_free> _listener(listener);
    evconnlistener_set_error_cb(&listener, on_error);

    return event_base_dispatch(&base) ? EXIT_SUCCESS : EXIT_FAILURE;
} catch(std::exception & e) {
    perror(e.what());
    exit(EXIT_FAILURE);
}

//
