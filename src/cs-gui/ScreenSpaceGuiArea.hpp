////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GUI_VISTA_SCREENSPACEGUIAREA_HPP
#define CS_GUI_VISTA_SCREENSPACEGUIAREA_HPP

#include "GuiArea.hpp"

#include <VistaAspects/VistaObserver.h>

class VistaViewport;

namespace cs::gui {

/// This class is used to render static UI elements, which are always at the same position of the
/// screen.
class CS_GUI_EXPORT ScreenSpaceGuiArea : public GuiArea,
                                         public IVistaObserver {

 public:
  explicit ScreenSpaceGuiArea(VistaViewport* pViewport);
  ~ScreenSpaceGuiArea() override = default;

  int getWidth() const override;
  int getHeight() const override;

  /// Handles changes to the screen size.
  void ObserverUpdate(IVistaObserveable* pObserveable, int nMsg, int nTicket) override;

 private:
  virtual void onViewportChange();

  VistaViewport*   mViewport;
  int              mWidth       = 0;
  int              mHeight      = 0;
};

} // namespace cs::gui

#endif // CS_GUI_VISTA_SCREENSPACEGUIAREA_HPP
