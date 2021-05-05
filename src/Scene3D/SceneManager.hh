/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <string>

#include <ignition/common/Console.hh>
#include <ignition/common/MeshManager.hh>

#include <ignition/math/Vector2.hh>
#include <ignition/math/Vector3.hh>

// TODO(louise) Remove these pragmas once ign-rendering and ign-msgs
// are disabling the warnings
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <ignition/msgs.hh>

#include <ignition/rendering/Camera.hh>
#include <ignition/rendering/OrbitViewController.hh>
#include <ignition/rendering/RayQuery.hh>
#include <ignition/rendering/RenderEngine.hh>
#include <ignition/rendering/RenderingIface.hh>
#include <ignition/rendering/Scene.hh>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace ignition
{
namespace gui
{
  /// \brief Scene manager class for loading and managing objects in the scene
  class SceneManager
  {
    /// \brief Constructor
    public: SceneManager();

    /// \brief Constructor
    /// \param[in] _scene Pointer to the rendering scene
    public: SceneManager(rendering::ScenePtr _scene);

    /// \brief Load the scene manager
    /// \param[in] _scene Pointer to the rendering scene
    public: void Load(rendering::ScenePtr _scene);

    /// \brief Update the scene based on pose msgs received
    public: void Update();

    public: rendering::ScenePtr GetScene();

    private: void ShowGrid();

    public: void SetActiveGrid(bool _grid);

    public: void LoadScene(const msgs::Scene &_msg);

    /// \brief Load the model from a model msg
    /// \param[in] _msg Model msg
    /// \return Model visual created from the msg
    public: rendering::VisualPtr LoadModel(const msgs::Model &_msg);

    /// \brief Load a link from a link msg
    /// \param[in] _msg Link msg
    /// \return Link visual created from the msg
    private: rendering::VisualPtr LoadLink(const msgs::Link &_msg);

    /// \brief Load a visual from a visual msg
    /// \param[in] _msg Visual msg
    /// \return Visual visual created from the msg
    private: rendering::VisualPtr LoadVisual(const msgs::Visual &_msg);

    /// \brief Load a geometry from a geometry msg
    /// \param[in] _msg Geometry msg
    /// \param[out] _scale Geometry scale that will be set based on msg param
    /// \param[out] _localPose Additional local pose to be applied after the
    /// visual's pose
    /// \return Geometry object created from the msg
    private: rendering::GeometryPtr LoadGeometry(const msgs::Geometry &_msg,
       math::Vector3d &_scale, math::Pose3d &_localPose);

    /// \brief Load a material from a material msg
    /// \param[in] _msg Material msg
    /// \return Material object created from the msg
    private: rendering::MaterialPtr LoadMaterial(const msgs::Material &_msg);

    /// \brief Load a light from a light msg
    /// \param[in] _msg Light msg
    /// \return Light object created from the msg
    private: rendering::LightPtr LoadLight(const msgs::Light &_msg);

    /// \brief Delete an entity
    /// \param[in] _entity Entity to delete
    private: void DeleteEntity(const unsigned int _entity);

    /// \brief Set the scene from a scene msg
    /// \param[in] _msg Scene msg
    public: void SetScene(const msgs::Scene &_scene);

    public: void UpdatePoses(std::unordered_map<long unsigned int, math::Pose3d> &_poses);

    //// \brief Pointer to the rendering scene
    private: rendering::ScenePtr scene;

    //// \brief Mutex to protect the pose msgs
    private: std::mutex mutex;

    /// \brief Map of entity id to pose
    private: std::unordered_map<long unsigned int, math::Pose3d> poses;

    /// \brief Map of entity id to initial local poses
    /// This is currently used to handle the normal vector in plane visuals. In
    /// general, this can be used to store any local transforms between the
    /// parent Visual and geometry.
    private: std::map<unsigned int, math::Pose3d> localPoses;

    /// \brief Map of visual id to visual pointers.
    private: std::map<unsigned int, rendering::VisualPtr::weak_type> visuals;

    /// \brief Map of light id to light pointers.
    private: std::map<unsigned int, rendering::LightPtr::weak_type> lights;

    /// Entities to be deleted
    private: std::vector<unsigned int> toDeleteEntities;

    /// \brief Keeps the a list of unprocessed scene messages
    private: std::vector<msgs::Scene> sceneMsgs;

    private: bool isgridactive = false;
    private: bool showGrid = false;
  };
}
}
