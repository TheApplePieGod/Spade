#if !defined(MAIN_H)

#include <stdio.h>    
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <new>
#include <Windows.h>
#include "cmath"

//
// Standard Type Definitions
// http://en.cppreference.com/w/c/types/integer	
// 
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int32_t b32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double d64;
typedef intptr_t intptr;
typedef uintptr_t uintptr;

#define global_var static

#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f
#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)
#define BITMAP_BYTES_PER_PIXEL 4
#define METERS_TO_PIXELS 100.0f

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;} // Dereference a NULL pointer
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define SAFE_RELEASE(Pointer) if(Pointer) {Pointer->Release();}

global_var int UserScreenSizeX = 1920;
global_var int UserScreenSizeY = 1080;

static void OutputDebugMessage(char InString[])
{
    char Buffer[256];
    _snprintf_s(Buffer, sizeof(Buffer), "%s\n", InString);
    OutputDebugStringA(Buffer);
}

static void OutputDebugFloat(f32 Data)
{
    char Buffer[256];
    _snprintf_s(Buffer, sizeof(Buffer), "%f\n", Data);
    OutputDebugStringA(Buffer);
}

static void OutputDebugVector(f32 x, f32 y, f32 z)
{
    char Buffer[256];
    _snprintf_s(Buffer, sizeof(Buffer), "X:%f, Y:%f, Z:%f\n", x, y, z);
    OutputDebugStringA(Buffer);
}

static void OutputDebugDouble(double Data)
{
    char Buffer[256];
    _snprintf_s(Buffer, sizeof(Buffer), "%g\n", Data);
    OutputDebugStringA(Buffer);
}

struct world_position
{
    f32 X;
    f32 Y;
    f32 Z;

    //v3 Offset; // From center
};

struct keyboard_button
{
    b32 WasPressed = false;
    b32 Pressed = false;
    u32 TransitionCount;
};

struct user_inputs // controller support later?
{
    // w a s d
    keyboard_button forward;
    keyboard_button backward;
    keyboard_button left;
    keyboard_button right;

    // right and left mouse buttons
    keyboard_button rmb;
    keyboard_button lmb;

    b32 MouseMovement = false;

    // view space
    f32 MousePosX;
    f32 MousePosY;

    float DeltaTime = 0;
};

#define MAIN_H
#endif