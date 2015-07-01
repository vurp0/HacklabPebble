#pragma once

#include "pebble.h"

static const GPathInfo MINUTE_HAND_POINTS = {
  4,
  (GPoint []) {
    { -2, 2 },
    { 2, 2 },
    { 2, -40 },
    { -2, -40 }
  }
};

static const GPathInfo HOUR_HAND_POINTS = {
  4, (GPoint []){
    {-2, 2},
    {2, 2},
    {2, -20},
    {-2, -20}
  }
};
