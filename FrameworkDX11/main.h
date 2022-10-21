#pragma once
#ifndef MAIN_H
#define MAIN_H

#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "Resource.h"

#include "Shaders.h"
#include "structures.h"
#include "ImGuiManager.h"
#include "DrawableGameObject.h"

#include "Sampler.h"
#include "Viewport.h"
#include "SwapChain.h"
#include "Rasterizer.h"
#include "RenderTarget.h"
#include "DepthStencil.h"

#include "Mouse.h"
#include "Camera.h"
#include "Keyboard.h"

typedef std::vector<DrawableGameObject*> vecDrawables;

#endif