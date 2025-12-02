
/*
 * buzzer.h
 * Non-blocking buzzer driver with tone + patterns
 */

#ifndef USER_INC_BUZZER_H_
#define USER_INC_BUZZER_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// High-level patterns
typedef enum {
    BUZZER_NONE = 0,
    BUZZER_DANGER,     // 3× 300ms @ 2kHz, 200ms gaps
    BUZZER_WARNING,    // 2× 200ms @ 1.5kHz, 300ms gaps
    BUZZER_BAD_INPUT,  // 1× 100ms @ 1kHz
    BUZZER_INFO,       // 1×  50ms @ 2.5kHz
    BUZZER_STARTUP,    // 2× 150ms @ 1.8kHz, 100ms gaps
    BUZZER_SHUTDOWN    // 1× 300ms @ 1.2kHz
} buzzer_pattern_t;

// Init / low-level control
bool  Buzzer_Init(void);                                  // starts PWM, mutes output
void  Buzzer_ToneStart(uint32_t freq_hz, float duty01);   // start/retune tone, non-blocking
void  Buzzer_ToneStop(void);                              // mute (duty=0)

// Non-blocking single beep (auto-stops after ms)
void  Buzzer_BeepNB(uint32_t freq_hz, uint32_t duration_ms);

// Patterns (non-blocking)
void  Buzzer_PlayPattern(buzzer_pattern_t pattern);

// Call often in main loop
void  Buzzer_Process(void);
void Buzzer_Test(void);
bool Buzzer_IsActive(void);
// Optional quick blocking beep (for quick tests only)
void  Buzzer_BeepBlocking(uint32_t freq_hz, uint32_t ms);

#ifdef __cplusplus
}
#endif
#endif /* USER_INC_BUZZER_H_ */
