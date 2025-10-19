#pragma once

#include <functional>

class Defer
{
public:
    Defer(std::function<void()> &func) : func_(std::move(func)) {}
    Defer(std::function<void()> &&func) : func_(std::move(func)) {}

    // 析构函数执行func_
    ~Defer() { func_(); }

private:
    std::function<void()> func_;
};