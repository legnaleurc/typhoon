/*
The MIT License (MIT)

Copyright (c) 2015 Wei-Cheng Pan<legnaleurc@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ioloop.hpp"

#include <unordered_map>
#include <thread>
#include <random>
#include <array>

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


using namespace typhoon;


class IOLoop::Private {
public:
    typedef std::unordered_map<std::thread::id, std::shared_ptr<IOLoop> > IOLoopTable;

    static IOLoopTable & ioloops() {
        static IOLoopTable loops;
        return loops;
    }

    Private(): loop(), gk(), timeouts(), timers() {
        std::random_device rd;
        std::array<std::random_device::result_type, std::mt19937_64::state_size> rawseeds;
        std::generate_n(rawseeds.data(), rawseeds.size(), std::ref(rd));
        std::seed_seq seeds(std::begin(rawseeds), std::end(rawseeds));
        std::mt19937_64 engine(seeds);
        this->gk = std::bind(std::uniform_int_distribution<IOLoop::Key>(), engine);
    }

    bool timer_next(IOLoop::Key key, const IOLoop::Callback & cb, uint64_t ms) {
        auto it = this->timers.find(key);
        if (it == this->timers.end()) {
            return false;
        }
        auto t = it->second;
        t->expires_from_now(boost::posix_time::milliseconds(ms));
        t->async_wait([this, key, cb, ms](const boost::system::error_code & e)->void {
            if (this->timer_next(key, cb, ms)) {
                cb();
            }
        });
        return true;
    }

    boost::asio::io_service loop;
    std::function<IOLoop::Key ()> gk;
    std::unordered_map<IOLoop::Key, std::shared_ptr<boost::asio::deadline_timer>> timeouts;
    std::unordered_map<IOLoop::Key, std::shared_ptr<boost::asio::deadline_timer>> timers;
};


void IOLoop::initialize() {
    Private::ioloops();
}

// TODO thread safety
IOLoop & IOLoop::current() {
    auto & ioloops = Private::ioloops();
    auto id = std::this_thread::get_id();
    auto it = ioloops.find(id);
    if (it != ioloops.end()) {
        return *it->second;
    }
    auto ioloop = std::shared_ptr<IOLoop>(new IOLoop, [](IOLoop * p)->void {
        delete p;
    });
    ioloops.insert(std::make_pair(id, ioloop));
    return *ioloop;
}

IOLoop::IOLoop(): p(new Private) {
}

IOLoop::~IOLoop() {
}

void IOLoop::start() {
    auto ok = this->p->loop.run();
    (void)ok;
}

void IOLoop::add_callback(const IOLoop::Callback & cb) {
    this->p->loop.post(cb);
}

// TODO deadline_timer leak?
IOLoop::Key IOLoop::add_timeout(const IOLoop::Callback & cb, uint64_t ms) {
    auto key = this->p->gk();
    auto t = std::make_shared<boost::asio::deadline_timer>(this->p->loop);
    this->p->timeouts.insert(std::make_pair(key, t));
    t->expires_from_now(boost::posix_time::milliseconds(ms));
    t->async_wait([this, key, cb](const boost::system::error_code & e)->void {
        this->p->timeouts.erase(key);
        cb();
    });
    return key;
}

bool IOLoop::remove_timeout(Key id) {
    auto it = this->p->timeouts.find(id);
    if (it == this->p->timeouts.end()) {
        return false;
    }
    it->second->cancel();
    this->p->timeouts.erase(it);
    return true;
}

IOLoop::Key IOLoop::add_timer(const IOLoop::Callback & cb, uint64_t ms) {
    auto key = this->p->gk();
    auto t = std::make_shared<boost::asio::deadline_timer>(this->p->loop);
    this->p->timers.insert(std::make_pair(key, t));
    this->p->timer_next(key, cb, ms);
    return key;
}

bool IOLoop::remove_timer(Key id) {
    auto it = this->p->timers.find(id);
    if (it == this->p->timers.end()) {
        return false;
    }
    it->second->cancel();
    this->p->timers.erase(it);
    return true;
}
