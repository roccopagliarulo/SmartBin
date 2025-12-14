// include/InputManager.h
#pragma once

// Define button pins
#define BTN_NONE 0
#define BTN_CONFIRM 1
#define BTN_CORRECT_PAPER 2
#define BTN_CORRECT_PLASTIC 3
#define BTN_CORRECT_GLASS 4   

#define BTN_CANCEL 5

void input_setup();
int checkButtons();