//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#ifndef _JGE_H_
#define _JGE_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <queue>
#include <map>

#include "JTypes.h"

#define DEBUG_PRINT

//#define _MP3_ENABLED_

#if defined(WIN32)
#include <windows.h>
typedef WPARAM LocalKeySym;
#define LOCAL_KEY_NONE ((WPARAM)-1)
#elif defined(LINUX)
#include <X11/XKBlib.h>
#include <X11/keysym.h>
typedef KeySym LocalKeySym;
#define LOCAL_KEY_NONE XK_VoidSymbol
#else
typedef u32 LocalKeySym;
#define LOCAL_KEY_NONE ((u32)-1)
#endif


bool JGEGetButtonState(const JButton button);
bool JGEGetButtonClick(const JButton button);
void JGECreateDefaultBindings();
int JGEGetTime();
u8 JGEGetAnalogX();
u8 JGEGetAnalogY();

#if !defined(WIN32) && !defined(LINUX)

#include <pspgu.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <time.h>
#include <string.h>
#include <pspaudiolib.h>
#include <psprtc.h>

#endif

//#include "JGEInit.h"

//#include "JTypes.h"

#include "Vector2D.h"

class JApp;
class JResourceManager;
class JFileSystem;
class JParticleSystem;
class JMotionSystem;
class JMusic;

//////////////////////////////////////////////////////////////////////////
/// Game engine main interface.
//////////////////////////////////////////////////////////////////////////
class JGE
{
 private:
  JApp *mApp;

#if defined (WIN32) || defined (LINUX)
  float mDeltaTime;
  JMusic *mCurrentMusic;
#else
 public:
  void Run();
 private:
#endif

  bool mDone;
  float mDelta;
  bool mDebug;
  bool mPaused;
  char mDebuggingMsg[256];
  bool mCriticalAssert;
  const char *mAssertFile;
  int mAssertLine;


  static JGE* mInstance;


  static std::queue< std::pair<std::pair<LocalKeySym, JButton>, bool> > keyBuffer;
  static std::multimap<LocalKeySym, JButton> keyBinds;
  typedef std::multimap<LocalKeySym, JButton>::iterator keycodes_it;

  friend void Run();

 public:

  //////////////////////////////////////////////////////////////////////////
  /// Get JGE instance.
  ///
  /// @return JGE instance.
  //////////////////////////////////////////////////////////////////////////
  static JGE* GetInstance();
  static void Destroy();

  void Init();
  void End();

  void Update(float);
  void Render();

  void Pause();
  void Resume();

  //////////////////////////////////////////////////////////////////////////
  /// Return system timer in milliseconds.
  ///
  /// @return System time in milliseconds.
  //////////////////////////////////////////////////////////////////////////
  int GetTime(void);

  //////////////////////////////////////////////////////////////////////////
  /// Return elapsed time since last frame update.
  ///
  /// @return Elapsed time in seconds.
  //////////////////////////////////////////////////////////////////////////
  float GetDelta();

  //////////////////////////////////////////////////////////////////////////
  /// Return frame rate.
  ///
  /// @note This is just 1.0f/GetDelta().
  ///
  /// @return Number of frames per second.
  //////////////////////////////////////////////////////////////////////////
  float GetFPS();

  //////////////////////////////////////////////////////////////////////////
  /// Check the current state of a button.
  ///
  /// @param button - Button id.
  ///
  /// @return Button state.
  //////////////////////////////////////////////////////////////////////////
  bool GetButtonState(JButton button);

  //////////////////////////////////////////////////////////////////////////
  /// Check if a button is down the first time.
  /// THIS DOES NOT WORK RELIABLY. DO NOT USE THIS.
  /// USE ReadButton() INSTEAD.
  ///
  /// @param button - Button id.
  ///
  /// @return Button state.
  //////////////////////////////////////////////////////////////////////////
  bool GetButtonClick(JButton button);

  //////////////////////////////////////////////////////////////////////////
  /// Get the next keypress.
  ///
  /// @return Next pressed button, or 0 if none.
  //////////////////////////////////////////////////////////////////////////
  JButton ReadButton();
  LocalKeySym ReadLocalKey();

  //////////////////////////////////////////////////////////////////////////
  /// Bind an actual key to a symbolic button. A key can be bound to
  /// several buttons and even several times to the same button (for
  /// double clicks on one button, for example.
  ///
  /// @param keycode - The local code of the key
  /// @param button - The button to bind it to
  ///
  /// @return The number of bound keys so far
  //////////////////////////////////////////////////////////////////////////
  u32 BindKey(LocalKeySym keycode, JButton button);

  //////////////////////////////////////////////////////////////////////////
  /// Undo a binding.
  /// If the second parameter is omitted, remove all bindings to this key ;
  /// else, remove exactly once the binding from this key to this button.
  ///
  /// @param keycode - The local code of the key
  /// @param button - The button
  ///
  /// @return The number of still bound keys
  //////////////////////////////////////////////////////////////////////////
  u32 UnbindKey(LocalKeySym keycode, JButton button);
  u32 UnbindKey(LocalKeySym keycode);

  //////////////////////////////////////////////////////////////////////////
  /// Clear bindings.
  /// This removes ALL bindings. Take care to re-bind keys after doing it.
  ///
  //////////////////////////////////////////////////////////////////////////
  void ClearBindings();

  //////////////////////////////////////////////////////////////////////////
  /// Reset bindings.
  /// This resets ALL bindings to their default value.
  ///
  //////////////////////////////////////////////////////////////////////////
  void ResetBindings();

  //////////////////////////////////////////////////////////////////////////
  /// Iterators for bindings.
  //////////////////////////////////////////////////////////////////////////
  typedef std::multimap<LocalKeySym, JButton>::const_iterator keybindings_it;
  keybindings_it KeyBindings_begin();
  keybindings_it KeyBindings_end();


  //////////////////////////////////////////////////////////////////////////
  /// Reset the input buffer.
  /// This is necessary because there might be phases when only
  /// GetButtonState is used, thereby accumulating keypresses
  /// in the key buffer.
  ///
  //////////////////////////////////////////////////////////////////////////
  void ResetInput();

  //////////////////////////////////////////////////////////////////////////
  /// Get x value of the analog pad.
  ///
  /// @return X value (0 to 255).
  //////////////////////////////////////////////////////////////////////////
  u8 GetAnalogX();

  //////////////////////////////////////////////////////////////////////////
  /// Get y value of the analog pad.
  ///
  /// @return Y value (0 to 255).
  //////////////////////////////////////////////////////////////////////////
  u8 GetAnalogY();

  //////////////////////////////////////////////////////////////////////////
  /// Simulate a keypress, or a keyhold/release.
  ///
  //////////////////////////////////////////////////////////////////////////
  void PressKey(const LocalKeySym);
  void PressKey(const JButton);
  void HoldKey(const LocalKeySym);
  void HoldKey(const JButton);
  void HoldKey_NoRepeat(const LocalKeySym);
  void HoldKey_NoRepeat(const JButton);
  void ReleaseKey(const LocalKeySym);
  void ReleaseKey(const JButton);



  //////////////////////////////////////////////////////////////////////////
  /// Get if the system is ended/paused or not.
  ///
  /// @return Status of the system.
  //////////////////////////////////////////////////////////////////////////
  bool IsDone() { return mDone; }
  bool IsPaused() { return mPaused; }


  //////////////////////////////////////////////////////////////////////////
  /// Set the user's core application class.
  ///
  /// @param app - User defined application class.
  //////////////////////////////////////////////////////////////////////////
  void SetApp(JApp *app);


  //////////////////////////////////////////////////////////////////////////
  /// Print debug message.
  ///
  //////////////////////////////////////////////////////////////////////////
  void printf(const char *format, ...);


  void Assert(const char *filename, long lineNumber);

#if defined (WIN32) || defined (LINUX)
  void SetDelta(float delta);
#endif

 protected:
  JGE();
  ~JGE();


};


#endif
