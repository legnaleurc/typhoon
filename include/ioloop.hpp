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

#ifndef TYPHOON_IOLOOP_HPP
#define TYPHOON_IOLOOP_HPP

#include <memory>
#include <functional>

namespace typhoon {

    class IOLoop {
    public:
        typedef uint64_t Key;
        typedef std::function<void ()> Callback;

        static void initialize();
        static IOLoop & current();

        void start();
        void stop();
        void close();

        void add_callback(const Callback & cb);
        Key add_timeout(const Callback & cb, uint64_t ms);
        bool remove_timeout(Key id);
        Key add_timer(const Callback & cb, uint64_t ms);
        bool remove_timer(Key id);

    private:
        IOLoop();
        ~IOLoop();
        IOLoop(const IOLoop &);
        IOLoop & operator =(const IOLoop &);

        class Private;
        friend class Private;
        std::shared_ptr<Private> p;
    };

}

#endif
