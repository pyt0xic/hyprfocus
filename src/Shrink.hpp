#pragma once
#include "IFocusAnimation.hpp"
#include <hyprland/src/helpers/AnimatedVariable.hpp>

class CShrink : public IFocusAnimation {
public:
  void onWindowFocus(PHLWINDOW pWindow, HANDLE pHandle) override;
  void init(HANDLE pHandle, std::string animationName) override;
  void setup(HANDLE pHandle, std::string animationName) override;

  PHLANIMVAR<float>               m_sShrinkAnimation;
  SP<Config::Values::CFloatValue> m_pShrinkPercentage;
};
