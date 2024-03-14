/*
 * reach_c_stack_version.h
 *  Version of reach core code
 */

#ifndef _REACH_VERSION_H_
#define _REACH_VERSION_H_

#define MAJOR_VERSION 2
#define MINOR_VERSION 5
#define BUILD_VERSION 2

#ifdef BUILT_BY_PIPELINE 
  #define PATCH_VERSION 0
#else
  #define PATCH_VERSION 255  // when built out of control
#endif

#endif  // ndef _REACH_VERSION_H_
