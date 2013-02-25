// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUGGEE_IPROCESS_H_
#define DEBUGGER_CORE_DEBUGGEE_IPROCESS_H_
#include <windows.h>
#include <deque>
#include <string>

/// \brief This namespace groups classes related to OOP (out-of-process)
/// Windows debugger.
namespace debug {
class Breakpoint;
class DebugAPI;
class DebuggeeThread;
class DebugEvent;

/// This abstract class represents a process in debugged application.
/// Inherited by DebuggeeProcess and DebuggeeProcessMock classes.

/// Class diagram (and more) is here:
/// https://docs.google.com/a/google.com/document/d/1lTN-IYqDd_oy9XQg9-zlNc_vbg-qyr4q2MKNEjhSA84/edit?hl=en&authkey=CJyJlOgF#
class IDebuggeeProcess {
 public:
  enum State {
    kRunning,  // process is alive, event loop is running
    kHalted,  // process is alive, event loop is not running
    kDead  // process is deleted by OS
  };

  IDebuggeeProcess() {}
  virtual ~IDebuggeeProcess() {}

  /// If enabled, it decrements IP after hitting breakpoint.
  virtual void EnableCompatibilityMode() = 0;
  virtual bool compatibility_mode() const = 0;

  /// @return id of the process
  virtual int id() const = 0;

  virtual State state() const = 0;
  virtual DebugAPI& debug_api() = 0;

  virtual bool IsHalted() const = 0;

  /// @return reference to last received debug event
  virtual const DebugEvent& last_debug_event() const = 0;

  /// @return address of memory region where nexe is loaded.
  virtual void* nexe_mem_base() const  = 0;

  virtual void set_nexe_mem_base(void* addr) = 0;

  /// @return code address of nexe _start() routine.
  virtual void* nexe_entry_point() const = 0;

  virtual void set_nexe_entry_point(void* addr) = 0;

  /// @return word size of the debuggee process (32 or 64).
  virtual int GetWordSizeInBits() = 0;

  /// @return true for WoW (windows-on-windows) processes -
  /// i.e. 32-bit processes running on 64-bit windows.
  virtual bool IsWoW() = 0;

  /// Allows process execution to continue (i.e. it calls
  /// ContinueDebugEvent() for halted thread).
  /// Shall be called only on halted process, and only from the thread that
  /// started the debuggee.
  virtual bool Continue() = 0;

  /// Allows process execution to continue. If thread was halted due
  /// to exception, that exception is passed to the debugee thread.
  /// Shall be called only on halted process, and only from the thread that
  /// started the debuggee.
  virtual bool ContinueAndPassExceptionToDebuggee() = 0;

  /// Cause halted thread to execute single CPU instruction.
  /// Shall be called only on halted process, and only from the thread that
  /// started the debuggee.
  virtual bool SingleStep() = 0;

  /// Cause running process to break (calls
  /// debug::DebugApi::DebugBreakProcess).
  /// Shall not be called on halted process, and only from the thread that
  /// started the debuggee.
  virtual bool Break() = 0;

  /// Terminates all threads of the process.
  /// Event loop should process exiting debug event before DebuggeeProcess
  /// object gets into kDead state and can be safely deleted.
  /// TODO(garianov): verify that |Kill| can be called from any thread.
  virtual bool Kill() = 0;

  /// Detaches debugger fom the process. Process is not killed.
  /// TODO(garianov): verify that |Detach| can be called from any thread.
  virtual bool Detach() = 0;

  /// @return a pointer to the thread object, or NULL if there's
  /// no thread with such |id|.
  /// Thread object is owned by the process.
  virtual DebuggeeThread* GetThread(int id) = 0;

  /// @return a poiner to the halted thread object, or NULL
  /// if process is not halted.
  /// Thread object is owned by the process.
  virtual DebuggeeThread* GetHaltedThread() = 0;

  /// @return all thread ids.
  virtual void GetThreadIds(std::deque<int>* threads) const = 0;

  /// Copies memory from debuggee process to debugger buffer.
  /// Shall be called only on halted process. There's no harm though if you
  /// call it on running process.
  /// @param[in] addr address (in debugger address space) from where to read.
  /// @param[in] size number of bytes to read.
  /// @param[out] destination destination buffer (in debugger address space).
  /// TODO(garianov): verify that |ReadMemory| can be called from any thread.
  virtual bool ReadMemory(const void* addr, size_t size, void* destination) = 0;

  /// Copies memory from debugger to debuggee process.
  /// Shall be called only on halted process.
  /// @param[in] addr address (in debugger address space) where to write.
  /// @param[in] size number of bytes to write.
  /// @param[in] source address of source buffer.
  /// TODO(garianov): verify that |WriteMemory| can be called from any thread.
  virtual bool WriteMemory(const void* addr,
                           size_t size,
                           const void* source) = 0;

  /// Reads string passed by OUTPUT_DEBUG_STRING_EVENT.
  /// Note: string data is located in debuggee process.
  /// Shall be called only on halted process.
  /// @param[out] debug_string destination for the string.
  /// @return true if last debug event was OUTPUT_DEBUG_STRING_EVENT and
  /// string data transfer from debuggee process was successful.
  virtual bool ReadDebugString(std::string* debug_string) = 0;

  /// Sets breakpoint at specified address |addr|.
  /// Shall be called only on halted process.
  /// Note: for NaCl threads, breakpoints are supported only in nexe code,
  /// i.e. breakpoints in TCB won't work.
  /// TODO(garianov): add support for breakpoints in TCB.
  /// @param[in] addr address where breakpoint shall be.
  /// @return false if process is not able to access memory at |addr|,
  /// or process is not halted, or if there is breakpoint with the same |addr|.
  virtual bool SetBreakpoint(void* addr) = 0;

  /// Removes breakpoint at specified address |addr|.
  /// Shall be called only on halted process.
  /// @param[in] addr address of breakpoint.
  /// @return false if process is not halted.
  virtual bool RemoveBreakpoint(void* addr) = 0;

  /// @return breakpoint object, or NULL if there's no breakpoint
  /// set at |addr|.
  /// @param[in] addr address of breakpoint.
  virtual Breakpoint* GetBreakpoint(void* addr) = 0;

  /// Return all breakpoints.
  /// @param[out] breakpoints
  virtual void GetBreakpoints(std::deque<Breakpoint*>* breakpoints) = 0;

  /// Converts relative pointer to flat(aka linear) process address.
  /// Calling this function makes sense only for nexe threads,
  /// it's safe to call for any thread.
  /// @param[in] ptr relative pointer
  /// @return flat address
  virtual void* FromNexeToFlatAddress(void* ptr) const = 0;

 private:
  friend class ExecutionEngine;

  /// Handler of debug events.
  /// @param[in] debug_event debug event received from debuggee process
  virtual void OnDebugEvent(DebugEvent* debug_event) = 0;

  IDebuggeeProcess(const IDebuggeeProcess&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const IDebuggeeProcess&);
};
}  // namespace debug
#endif  // DEBUGGER_CORE_DEBUGGEE_IPROCESS_H_
