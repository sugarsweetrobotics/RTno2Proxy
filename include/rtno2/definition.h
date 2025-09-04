#pragma once

#include <string>
#include <sstream>

#include "result.h"

// Communication Settings
#define PACKET_WAITING_TIME 3000 // ms
#define PACKET_SENDING_DELAY 1 // us
#define PACKET_WAITING_DELAY 100 //us
#define PACKET_WAITING_COUNT (PACKET_WAITING_TIME*1000/PACKET_WAITING_DELAY)

#define MAX_PACKET_SIZE 64

