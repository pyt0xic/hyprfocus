#pragma once
#include "IFocusAnimation.hpp"

class CFlash : public IFocusAnimation {
public:
  void onWindowFocus(PHLWINDOW pWindow, HANDLE pHandle) override;
  void init(HANDLE pHandle, std::string animationName) override;
  void setup(HANDLE pHandle, std::string animationName) override;

  SP<Config::Values::CFloatValue> m_pFlashOpacity;
};
