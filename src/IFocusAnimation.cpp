#include "IFocusAnimation.hpp"
#include "Globals.hpp"

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/config/shared/animation/AnimationTree.hpp>
#include <hyprlang.hpp>

SP<Config::Values::CFloatValue>
IFocusAnimation::registerFloat(HANDLE pHandle, const std::string &name,
                               float def) {
  m_configNames.emplace_back(name);
  auto v = makeShared<Config::Values::CFloatValue>(
      m_configNames.back().c_str(), "hyprfocus plugin value",
      (Config::FLOAT)def);
  HyprlandAPI::addConfigValueV2(pHandle, v);
  return v;
}

SP<Config::Values::CStringValue>
IFocusAnimation::registerString(HANDLE pHandle, const std::string &name,
                                const std::string &def) {
  m_configNames.emplace_back(name);
  auto v = makeShared<Config::Values::CStringValue>(
      m_configNames.back().c_str(), "hyprfocus plugin value", def);
  HyprlandAPI::addConfigValueV2(pHandle, v);
  return v;
}

void IFocusAnimation::init(HANDLE pHandle, std::string animationName) {
  m_szAnimationName = animationName;
  hyprfocus_log(LOG, "Initializing focus animation: {}", animationName);

  m_pInBezier  = registerString(pHandle, configPrefix() + "in_bezier", "default");
  m_pOutBezier = registerString(pHandle, configPrefix() + "out_bezier", "default");
  m_pInSpeed   = registerFloat(pHandle, configPrefix() + "in_speed", 1.f);
  m_pOutSpeed  = registerFloat(pHandle, configPrefix() + "out_speed", 5.f);

  m_sFocusInAnimConfig = makeShared<SAnimationPropertyConfig>();
  m_sFocusInAnimConfig->internalEnabled = 1;
  m_sFocusInAnimConfig->internalStyle =
      std::string("hyprfocus_") + animationName + std::string("_in");
  m_sFocusInAnimConfig->pValues = m_sFocusInAnimConfig;

  m_sFocusOutAnimConfig = makeShared<SAnimationPropertyConfig>();
  m_sFocusOutAnimConfig->internalEnabled = 1;
  m_sFocusOutAnimConfig->internalStyle =
      std::string("hyprfocus_") + animationName + std::string("_out");
  m_sFocusOutAnimConfig->pValues = m_sFocusOutAnimConfig;
}

void IFocusAnimation::setup(HANDLE pHandle, std::string animationName) {
  // hyprfocus_log(LOG, "Setting up focus animation: {}", animationName);
}

void IFocusAnimation::onWindowFocus(PHLWINDOW pWindow, HANDLE pHandle) {
  hyprfocus_log(LOG, "Base callback for animation: {}", m_szAnimationName);
  if (!m_pInBezier || !m_pOutBezier || !m_pInSpeed || !m_pOutSpeed) {
    hyprfocus_log(ERR, "Config values not registered for animation {}",
                  m_szAnimationName);
    return;
  }

  m_sFocusInAnimConfig->internalBezier = m_pInBezier->value();
  m_sFocusInAnimConfig->internalSpeed  = m_pInSpeed->value();

  m_sFocusOutAnimConfig->internalBezier = m_pOutBezier->value();
  m_sFocusOutAnimConfig->internalSpeed  = m_pOutSpeed->value();

  hyprfocus_log(LOG, "In bezier: {} In speed: {} Out bezier: {} Out speed: {}",
                m_sFocusInAnimConfig->internalBezier,
                m_sFocusInAnimConfig->internalSpeed,
                m_sFocusOutAnimConfig->internalBezier,
                m_sFocusOutAnimConfig->internalSpeed);
}
