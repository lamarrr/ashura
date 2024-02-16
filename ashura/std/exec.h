#pragma once

namespace ash
{

template <typename T>
concept Sender = requires(T) {};

template <typename T>
concept Receiver = requires(T) {};


template <typename T>
concept ExecutionContext = requires(T) {};

template <typename T>
concept Executor = requires(T) {};

}        // namespace ash
