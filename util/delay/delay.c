/**
 * @file    delay.c
 * @author  Ryosuke Matsui
 * @version V1.0.0
 * @date    2017/02/11
 * @brief
 */

/* Includes ------------------------------------------------------------------*/
#include "delay.h"

// #include "platform_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//! 強制インライン化するためのマクロ
#define ALWAYS_INLINE __attribute__((always_inline))
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t _firstRun = 1; ///< 初回実行かどうかを判定する変数
/* Private function prototypes -----------------------------------------------*/
static inline void _delay_startCycleCount(void) ALWAYS_INLINE;
static inline void _delay_cycles(uint32_t cycles, uint32_t start) ALWAYS_INLINE;
inline void delay_usFromCount(uint32_t us, uint32_t startCount) ALWAYS_INLINE;
inline void delay_msFromCount(uint32_t ms, uint32_t startCount) ALWAYS_INLINE;
inline uint32_t delay_getCount(void) ALWAYS_INLINE;

/* Private functions ---------------------------------------------------------*/
/**
 * @brief CPUの動作サイクルカウンタ(CYCCNT)を有効化する関数(初回のみ)
 */
static inline void _delay_startCycleCount(void) {
	if (_firstRun) {
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
		DWT->CYCCNT = 0;
		_firstRun = 0;
	}
}

/**
 * @brief 指定したサイクル(カウント)数待つ関数
 * @param cycles 待つサイクル数 カウンタは32bitだが判定の都合上半分の2^31-1が最大値
 * @param startCount 基準となるカウント数
 */
static inline void _delay_cycles(uint32_t cycles, uint32_t startCount) {
	uint32_t end = startCount + cycles;

	while ((int32_t)(end - DWT->CYCCNT) > 0)
		continue;
}

/**
 * @brief 指定した時間[us]待つ関数
 *
 * 一応割り込み等と多重に使っても問題はない(そもそも割り込みでdelayを使うこと自体非推奨)
 * @param us 待つ時間[us]
 */
void delay_us(uint32_t us) {
	delay_usFromCount(us, delay_getCount());
}

/**
 * @brief ある基準の時から指定した時間[us]待つ関数
 *
 * 基準時のカウントはdelay_getCount関数を使って取得できる
 *
 * 使用例(100us間隔で特定の処理を実行):
 * @code
 * while(1) {
 *     uint32_t delayStartCount = delay_getCount();
 *     // {時間の掛かる処理}
 *     delay_usFromCount(100, delayStartCount);
 * }
 * @endcode
 * @param us 待つ時間[us]
 * @param startCount 基準時のカウント delay_getCount関数を使って得られるカウントを使用
 */
void delay_usFromCount(uint32_t us, uint32_t startCount) {
	_delay_cycles(SystemCoreClock / (1000 * 1000) * us, startCount);
}

/**
 * @brief 指定した時間[ms]待つ関数
 *
 * 一応割り込み等と多重に使っても問題はない(そもそも割り込みでdelayを使うこと自体非推奨)
 * 少なくとも10秒位は使える
 *
 * 最大値の計算例(動作周波数72MHz): 2^31(カウンタ最大値) / (72 * 1000^2) ≒ 29.8[s]
 * @param ms 待つ時間[ms]
 */
void delay_ms(uint32_t ms) {
	delay_msFromCount(ms, delay_getCount());
}

/**
 * @brief ある基準の時から指定した時間[ms]待つ関数
 *
 * 基準時のカウントはdelay_getCount関数を使って取得できる
 *
 * 使用例(100ms間隔で特定の処理を実行):
 * @code
 * while(1) {
 *     uint32_t delayStartCount = delay_getCount();
 *     // {時間の掛かる処理}
 *     delay_msFromCount(100, delayStartCount);
 * }
 * @endcode
 * @param ms 待つ時間[ms]
 * @param startCount 基準時のカウント delay_getCount関数を使って得られるカウントを使用
 */
void delay_msFromCount(uint32_t ms, uint32_t startCount) {
	_delay_cycles(SystemCoreClock / 1000 * ms, startCount);
}

/**
 * @brief 関数を実行した時のカウンタを取得する関数
 *
 * delay_usFromCountやdelay_msFromCount関数で使用
 *
 * 動作の処理時間計測にも利用可能(これがCYCCNTの本来の用途)
 * @code
 * uint32_t startTime = delay_getCount();
 * // {処理}
 * float processTime = (float)(delay_getCount() - startTime) / SystemCoreClock; // 処理時間[s]
 * @endcode
 * @return 関数実行時の内部サイクル数(CYCCNT)
 */
uint32_t delay_getCount(void) {
	_delay_startCycleCount();

	return DWT->CYCCNT;
}
