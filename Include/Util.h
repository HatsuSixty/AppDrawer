#pragma once

#define TRY(expr)                                       \
    try {                                               \
        expr;                                           \
    } catch (const std::runtime_error& ex) {            \
        std::cerr << ex.what() << "\n";                 \
        std::exit(1);                                   \
    }
