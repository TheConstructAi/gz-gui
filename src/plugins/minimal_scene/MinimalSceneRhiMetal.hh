/*
 * Copyright (C) 2022 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef IGNITION_GUI_PLUGINS_MINIMALSCENE_MINIMALSCENERHIMETAL_HH_
#define IGNITION_GUI_PLUGINS_MINIMALSCENE_MINIMALSCENERHIMETAL_HH_

#include "MinimalSceneRhi.hh"
#include "ignition/gui/Plugin.hh"

#include <QQuickWindow>
#include <QSGTexture>
#include <QSize>

#include <memory>

namespace ignition
{
namespace gui
{
namespace plugins
{
  /// \brief Private data for IgnCameraTextureRhiMetal
  class IgnCameraTextureRhiMetalPrivate;

  /// \brief Implementation of IgnCameraTextureRhi for the Metal graphics API
  class IgnCameraTextureRhiMetal : public IgnCameraTextureRhi
  {
    // Documentation inherited
    public: virtual ~IgnCameraTextureRhiMetal() override;

    /// \brief Constructor
    public: IgnCameraTextureRhiMetal();

    // Documentation inherited
    public: virtual void Update(rendering::CameraPtr _camera) override;

    // Documentation inherited
    public: virtual void TextureId(void* _texturePtr) override;

    /// \internal Pointer to private data
    private: std::unique_ptr<IgnCameraTextureRhiMetalPrivate> dataPtr;
  };

  /// \brief Private data for RenderThreadRhiMetal
  class RenderThreadRhiMetalPrivate;

  /// \brief Implementation of RenderThreadRhi for the Metal graphics API
  class RenderThreadRhiMetal : public RenderThreadRhi
  {
    // Documentation inherited
    public: virtual ~RenderThreadRhiMetal() override;

    /// \brief Constructor
    /// \param[in] _renderer The Ign-rendering renderer
    public: RenderThreadRhiMetal(IgnRenderer *_renderer);

    // Documentation inherited
    public: virtual void Initialize() override;

    // Documentation inherited
    public: virtual void RenderNext(RenderSync *_renderSync) override;

    // Documentation inherited
    public: virtual void* TexturePtr() const override;

    // Documentation inherited
    public: virtual QSize TextureSize() const override;

    // Documentation inherited
    public: virtual void ShutDown() override;

    /// \internal Prevent copy and assignment
    private: RenderThreadRhiMetal(
        const RenderThreadRhiMetal &_other) = delete;
    private: RenderThreadRhiMetal& operator=(
        const RenderThreadRhiMetal &_other) = delete;

    /// \internal Pointer to private data
    private: std::unique_ptr<RenderThreadRhiMetalPrivate> dataPtr;
  };

  /// \brief Private data for TextureNodeRhiMetal
  class TextureNodeRhiMetalPrivate;

  /// \brief Implementation of TextureNodeRhi for the Metal graphics API
  class TextureNodeRhiMetal : public TextureNodeRhi
  {
    // Documentation inherited
    public: virtual ~TextureNodeRhiMetal() override;

    /// \brief Constructor
    /// \param[in] _window Window to display the texture
    public: TextureNodeRhiMetal(QQuickWindow *_window);

    // Documentation inherited
    public: virtual QSGTexture *Texture() const override;

    // Documentation inherited
    public: virtual bool HasNewTexture() const override;

    // Documentation inherited
    public: virtual void NewTexture(
        void* _texturePtr, const QSize &_size)override;

    // Documentation inherited
    public: virtual void PrepareNode() override;

    /// \internal Prevent copy and assignment
    private: TextureNodeRhiMetal(
        const TextureNodeRhiMetal &_other) = delete;
    private: TextureNodeRhiMetal& operator=(
        const TextureNodeRhiMetal &_other) = delete;

    /// \internal Pointer to private data
    private: std::unique_ptr<TextureNodeRhiMetalPrivate> dataPtr;
   };
}
}
}

#endif
