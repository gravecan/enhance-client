#pragma once

#include <Windows.h>
#include <vector>
#include <memory>
#include <jni.h>
#include <thread>
#include <iostream>
#include <mutex>
#include <gl/GL.h>

#include "../utils/minhook/include/MinHook.h"
#include "../utils/jnihook-master/include/jnihook.h"
#include "../utils/imgui/imgui.h"
#include "../utils/imgui/imgui_impl_opengl3.h"
#include "../utils/imgui/imgui_impl_win32.h"
#include "mappings/mappings.hpp"