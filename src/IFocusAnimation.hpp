#pragma once

#include <hyprlang.hpp>
#define WLR_USE_UNSTABLE

#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/config/values/types/FloatValue.hpp>
#include <hyprland/src/config/values/types/StringValue.hpp>
#include <hyprutils/animation/AnimationConfig.hpp>

using namespace Hyprutils::Memory;
using namespace Hyprutils::Animation;

class IFocusAnimation {
public:
  virtual ~IFocusAnimation() = default;
  virtual void onWindowFocus(PHLWINDOW pWindow, HANDLE pHandle);
  virtual void init(HANDLE pHandle, std::string animationName);
  virtual void setup(HANDLE pHandle, std::string animationName);

  // V2 API: register a plugin float value owned by this animation.
  // Returns a shared pointer to the value object. Name string is kept alive
  // by storing it in m_configNames.
  SP<Config::Values::CFloatValue> registerFloat(HANDLE pHandle,
                                                const std::string &name,
                                                float def);
  SP<Config::Values::CStringValue> registerString(HANDLE pHandle,
                                                  const std::string &name,
                                                  const std::string &def);

public:
  SP<SAnimationPropertyConfig> m_sFocusInAnimConfig;
  SP<SAnimationPropertyConfig> m_sFocusOutAnimConfig;

  std::string m_szAnimationName;

  // Per-animation config values (V2).
  SP<Config::Values::CStringValue> m_pInBezier;
  SP<Config::Values::CStringValue> m_pOutBezier;
  SP<Config::Values::CFloatValue>  m_pInSpeed;
  SP<Config::Values::CFloatValue>  m_pOutSpeed;

  // String storage so the const char* in the IValue stays valid.
  std::vector<std::string> m_configNames;

  std::string configPrefix() {
    return std::string("plugin:hyprfocus:") + m_szAnimationName + ":";
  }
};
