#pragma once

#include <functional>

class Defer
{
public:
    Defer(const std::function<void()> &func) : func_(func) {}
    Defer(std::function<void()> &&func) : func_(std::move(func)) {}

    ~Defer() { func_(); }

private:
    std::function<void()> func_;
};