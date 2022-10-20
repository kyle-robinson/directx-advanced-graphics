#pragma once
#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "Resource.h"
#include <iostream>
#include <memory>

#include "ErrorLogger.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Camera.h"
#include "DrawableGameObject.h"
#include "structures.h"
#include <vector>

typedef std::vector<DrawableGameObject*> vecDrawables;

#endif