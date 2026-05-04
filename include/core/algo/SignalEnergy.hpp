#pragma once

#include <cstddef>
#include <cstdint>

namespace ArmProject::Algo
{
    /// @brief Скалярная (эталонная) реализация.
    /// @param data Указатель на массив 32-битных знаковых отсчётов.
    /// @param n Количество элементов.
    /// @return Сумма модулей элементов в виде int64_t.
    std::int64_t SignalEnergyScalar(const std::int32_t* data, std::size_t n) noexcept;

    /// @brief NEON-реализация (на не-ARM платформах макрос подставляет скалярную).
    std::int64_t SignalEnergyNeon(const std::int32_t* data, std::size_t n) noexcept;

    /// @brief NEON-реализация с разворотом цикла (8 элементов за итерацию).
    std::int64_t SignalEnergyNeonUnrolled(const std::int32_t* data, std::size_t n) noexcept;
}
