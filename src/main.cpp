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

#include <iostream>

#include "ioloop.hpp"


int main (int argc, char * argv[]) {
    using typhoon::IOLoop;

    IOLoop::initialize();

    auto & main_loop = IOLoop::current();

    int i = 5;
    IOLoop::Key j = 0ULL;

    main_loop.add_callback([]()->void {
        std::cout << "later" << std::endl;
    });
    main_loop.add_timeout([]()->void {
        std::cout << "after 1 second" << std::endl;
    }, 1000);
    j = main_loop.add_timer([&i, &j, &main_loop]()->void {
        std::cout << "periodic " << i << std::endl;
        if (i > 0) {
            --i;
        } else {
            main_loop.remove_timer(j);
        }
    }, 500);
    std::cout << "start" << std::endl;

    main_loop.start();

    return 0;
}
