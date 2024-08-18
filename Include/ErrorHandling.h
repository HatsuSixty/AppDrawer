#ifndef ERROR_HANDLING_H_
#define ERROR_HANDLING_H_

#include <cassert>

template<typename E, typename V>
class Result {
private:
    V* value;
    E* error;

public:
    static Result fromValue(V value);
    static Result fromError(E error);

    ~Result();

    bool isOk() const;
    V getValue() const;
    E getError() const;

    template<typename F>
    Result onError(F&& function);

    template<typename F>
    Result onValue(F&& function);
};

#ifdef ERROR_HANDLING_IMPLEMENTATION

template<typename E, typename V>
Result<E, V> Result<E, V>::fromValue(V value)
{
    value = malloc(sizeof(V));
    error = nullptr;
}

template<typename E, typename V>
Result<E, V> Result<E, V>::fromError(E error)
{
    error = malloc(sizeof(E));
    value = nullptr;
}

template<typename E, typename V>
Result<E, V>::~Result()
{
    if (value)
        free(value);
    if (error)
        free(error);
}

template<typename E, typename V>
bool Result<E, V>::isOk() const
{
    return value != nullptr;
}

template<typename E, typename V>
V Result<E, V>::getValue() const
{
    assert(value);
    return *value;
}

template<typename E, typename V>
E Result<E, V>::getError() const
{
    assert(error);
    return *error;
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

#endif // ERROR_HANDLING_IMPLEMENTATION

#endif // ERROR_HANDLING_H
