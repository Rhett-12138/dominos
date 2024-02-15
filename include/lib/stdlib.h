#pragma once
#include <types.h>

#define MAX(a, b) (a < b ? b : a)
#define MIN(a, b) (a < b ? a : b)

void delay(uint32_t count);
void hang();

uint8_t bcd_to_bin(uint8_t value);
uint8_t bin_to_bcd(uint8_t value);

uint32_t div_round_up(uint32_t num, uint32_t size);