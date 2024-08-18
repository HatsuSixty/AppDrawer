#ifndef ERROR_HANDLING_H_
#define ERROR_HANDLING_H_

#include <cassert>
#include <cstdlib>

template<typename E, typename V>
class Result {
private:
    bool isError;
    V value;
    E error;

public:
    static Result fromValue(V value);
    static Result fromError(E error);

    bool isOk() const;
    V getValue() const;
    E getError() const;

    template<typename F>
    Result onError(F&& function);

    template<typename F>
    Result onValue(F&& function);
};

template<typename E, typename V>
Result<E, V> Result<E, V>::fromValue(V value)
{
    Result<E, V> result;
    result.value = value;
    result.isError = false;
    return result;
}

template<typename E, typename V>
Result<E, V> Result<E, V>::fromError(E error)
{
    Result<E, V> result;
    result.error = error;
    result.isError = true;
    return result;
}

template<typename E, typename V>
bool Result<E, V>::isOk() const
{
    return !isError;
}

template<typename E, typename V>
V Result<E, V>::getValue() const
{
    assert(isOk());
    return value;
}

template<typename E, typename V>
E Result<E, V>::getError() const
{
    assert(!isOk());
    return error;
}

template<typename E, typename V>
template<typename F>
Result<E, V> Result<E, V>::onError(F&& function)
{
    if (!isOk()) function();
    return *this;
}

template<typename E, typename V>
template<typename F>
Result<E, V> Result<E, V>::onValue(F&& function)
{
    if (isOk()) function();
    return *this;
}

#endif // ERROR_HANDLING_H
