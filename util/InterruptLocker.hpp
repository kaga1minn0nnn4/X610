/**
 * @file    InterruptLocker.hpp
 * @author  public26
 * @version V1.0.0
 * @date    2017/11/25
 * @brief
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UTIL_INTERRUPTLOCKER_HPP_
#define UTIL_INTERRUPTLOCKER_HPP_

/* Includes ------------------------------------------------------------------*/
#include <cstddef>
#include "stm32g4xx.h"

/**
 * @brief 割り込み禁止用クラス
 * @details RAIIとして使ってください
 * @code
 * {// 割り込みを禁止したい処理のブロック
 *     InterruptLocker locker;
 *     ~~~
 * }
 * @endcode
 * @attention このクラスを用いずに割り込み禁止を行ってしまうと破錠してしまうので注意
 */
class InterruptLocker
{
public:
	// Non Copyable
	InterruptLocker(const InterruptLocker&) = delete;
	InterruptLocker& operator=(const InterruptLocker&) = delete;
	void* operator new(std::size_t) = delete;

public:
	inline InterruptLocker() {
		__disable_irq();
		stack++;
	}
	inline ~InterruptLocker() {
		if (--stack == 0) {
			__enable_irq();
		}
	}

private:
	static inline volatile uint32_t stack = 0;
};

#endif /* UTIL_INTERRUPTLOCKER_HPP_ */
