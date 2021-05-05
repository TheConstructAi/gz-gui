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

#include "SceneManager.hh"
#include "ignition/rendering/Capsule.hh"
#include <ignition/rendering/Visual.hh>
#include <ignition/rendering/Geometry.hh>
#include <ignition/rendering/Grid.hh>

using namespace ignition;
using namespace gui;

/////////////////////////////////////////////////
SceneManager::SceneManager()
{
}

/////////////////////////////////////////////////
SceneManager::SceneManager(rendering::ScenePtr _scene)
{
  this->Load(_scene);
}

/////////////////////////////////////////////////
void SceneManager::Load(rendering::ScenePtr _scene)
{
  this->scene = _scene;
}

rendering::ScenePtr SceneManager::GetScene()
{
  return this->scene;
}

/////////////////////////////////////////////////
void SceneManager::Update()
{
  // process msgs
  std::lock_guard<std::mutex> lock(this->mutex);
  if (!this->isgridactive && this->showGrid)
  {
    ShowGrid();
    this->isgridactive = true;
  }

  for (const auto &msg : this->sceneMsgs)
  {
    this->LoadScene(msg);
  }
  this->sceneMsgs.clear();

  for (const auto &pose : this->poses)
  {
    auto it = this->visuals.find(pose.first);
    if (it != this->visuals.end())
    {
      it->second.lock()->SetLocalPose(pose.second);
    }
  }


}

void SceneManager::SetScene(const msgs::Scene &_scene)
{
  std::lock_guard<std::mutex> lock(this->mutex);
  this->sceneMsgs.push_back(_scene);
}

void SceneManager::SetActiveGrid(bool _grid)
{
  this->showGrid = _grid;
}

void SceneManager::ShowGrid()
{
  if (!this->scene)
  {
    return;
  }

  rendering::VisualPtr root = this->scene->RootVisual();

  // create gray material
  rendering::MaterialPtr gray = this->scene->CreateMaterial();
  gray->SetAmbient(0.7, 0.7, 0.7);
  gray->SetDiffuse(0.7, 0.7, 0.7);
  gray->SetSpecular(0.7, 0.7, 0.7);

  // create grid visual
  rendering::VisualPtr visual = this->scene->CreateVisual();

  rendering::GridPtr gridGeom = this->scene->CreateGrid();

  if (!gridGeom)
  {
    ignwarn << "Failed to create grid for scene ["
      << this->scene->Name() << "] on engine ["
        << this->scene->Engine()->Name() << "]"
          << std::endl;
    return;
  }
  gridGeom->SetCellCount(20);
  gridGeom->SetCellLength(1);
  gridGeom->SetVerticalCellCount(0);
  visual->AddGeometry(gridGeom);
  visual->SetLocalPosition(0, 0, 0.015);
  visual->SetMaterial(gray);
  root->AddChild(visual);
}

void SceneManager::LoadScene(const msgs::Scene &_msg)
{
  if (!this->scene)
  {
    return;
  }
  rendering::VisualPtr rootVis = this->scene->RootVisual();

  // load models
  for (int i = 0; i < _msg.model_size(); ++i)
  {
    // Only add if it's not already loaded
    if (this->visuals.find(_msg.model(i).id()) == this->visuals.end())
    {
      rendering::VisualPtr modelVis = this->LoadModel(_msg.model(i));
      if (modelVis)
        rootVis->AddChild(modelVis);
      else
        ignerr << "Failed to load model: " << _msg.model(i).name() << std::endl;
    }
  }

  // load lights
  for (int i = 0; i < _msg.light_size(); ++i)
  {
    if (this->lights.find(_msg.light(i).id()) == this->lights.end())
    {
      rendering::LightPtr light = this->LoadLight(_msg.light(i));
      if (light)
        rootVis->AddChild(light);
      else
        ignerr << "Failed to load light: " << _msg.light(i).name() << std::endl;
    }
  }
}

/////////////////////////////////////////////////
rendering::VisualPtr SceneManager::LoadModel(const msgs::Model &_msg)
{
  rendering::VisualPtr modelVis = this->scene->CreateVisual(_msg.name());
  if (_msg.has_pose())
    modelVis->SetLocalPose(msgs::Convert(_msg.pose()));
  this->visuals[_msg.id()] = modelVis;

  // load links
  for (int i = 0; i < _msg.link_size(); ++i)
  {
    rendering::VisualPtr linkVis = this->LoadLink(_msg.link(i));
    if (linkVis)
      modelVis->AddChild(linkVis);
    else
      ignerr << "Failed to load link: " << _msg.link(i).name() << std::endl;
  }

  // load nested models
  for (int i = 0; i < _msg.model_size(); ++i)
  {
    rendering::VisualPtr nestedModelVis = this->LoadModel(_msg.model(i));
    if (nestedModelVis)
      modelVis->AddChild(nestedModelVis);
    else
      ignerr << "Failed to load nested model: " << _msg.model(i).name()
             << std::endl;
  }

  return modelVis;
}

/////////////////////////////////////////////////
rendering::VisualPtr SceneManager::LoadLink(const msgs::Link &_msg)
{
  rendering::VisualPtr linkVis = this->scene->CreateVisual();
  if (_msg.has_pose())
    linkVis->SetLocalPose(msgs::Convert(_msg.pose()));
  this->visuals[_msg.id()] = linkVis;

  // load visuals
  for (int i = 0; i < _msg.visual_size(); ++i)
  {
    rendering::VisualPtr visualVis = this->LoadVisual(_msg.visual(i));
    if (visualVis)
      linkVis->AddChild(visualVis);
    else
      ignerr << "Failed to load visual: " << _msg.visual(i).name() << std::endl;
  }

  // load lights
  for (int i = 0; i < _msg.light_size(); ++i)
  {
    rendering::LightPtr light = this->LoadLight(_msg.light(i));
    if (light)
      linkVis->AddChild(light);
    else
      ignerr << "Failed to load light: " << _msg.light(i).name() << std::endl;
  }

  return linkVis;
}

/////////////////////////////////////////////////
rendering::VisualPtr SceneManager::LoadVisual(const msgs::Visual &_msg)
{
  if (!_msg.has_geometry())
    return rendering::VisualPtr();

  rendering::VisualPtr visualVis = this->scene->CreateVisual();
  this->visuals[_msg.id()] = visualVis;

  math::Vector3d scale = math::Vector3d::One;
  math::Pose3d localPose;
  rendering::GeometryPtr geom =
      this->LoadGeometry(_msg.geometry(), scale, localPose);

  if (_msg.has_pose())
    visualVis->SetLocalPose(msgs::Convert(_msg.pose()) * localPose);
  else
    visualVis->SetLocalPose(localPose);

  if (geom)
  {
    // store the local pose
    this->localPoses[_msg.id()] = localPose;

    visualVis->AddGeometry(geom);
    visualVis->SetLocalScale(scale);

    // set material
    rendering::MaterialPtr material{nullptr};
    if (_msg.has_material())
    {
      material = this->LoadMaterial(_msg.material());
    }
    // Don't set a default material for meshes because they
    // may have their own
    // TODO(anyone) support overriding mesh material
    else if (!_msg.geometry().has_mesh())
    {
      // create default material
      material = this->scene->Material("ign-grey");
      if (!material)
      {
        material = this->scene->CreateMaterial("ign-grey");
        material->SetAmbient(0.3, 0.3, 0.3);
        material->SetDiffuse(0.7, 0.7, 0.7);
        material->SetSpecular(1.0, 1.0, 1.0);
        material->SetRoughness(0.2f);
        material->SetMetalness(1.0f);
      }
    }
    else
    {
      // meshes created by mesh loader may have their own materials
      // update/override their properties based on input sdf element values
      auto mesh = std::dynamic_pointer_cast<rendering::Mesh>(geom);
      for (unsigned int i = 0; i < mesh->SubMeshCount(); ++i)
      {
        auto submesh = mesh->SubMeshByIndex(i);
        auto submeshMat = submesh->Material();
        if (submeshMat)
        {
          double productAlpha = (1.0-_msg.transparency()) *
              (1.0 - submeshMat->Transparency());
          submeshMat->SetTransparency(1 - productAlpha);
          submeshMat->SetCastShadows(_msg.cast_shadows());
        }
      }
    }

    if (material)
    {
      // set transparency
      material->SetTransparency(_msg.transparency());

      // cast shadows
      material->SetCastShadows(_msg.cast_shadows());

      geom->SetMaterial(material);
      // todo(anyone) SetMaterial function clones the input material.
      // but does not take ownership of it so we need to destroy it here.
      // This is not ideal. We should let ign-rendering handle the lifetime
      // of this material
      this->scene->DestroyMaterial(material);
    }
  }
  else
  {
    ignerr << "Failed to load geometry for visual: " << _msg.name()
           << std::endl;
  }

  return visualVis;
}

/////////////////////////////////////////////////
rendering::GeometryPtr SceneManager::LoadGeometry(const msgs::Geometry &_msg,
    math::Vector3d &_scale, math::Pose3d &_localPose)
{
  math::Vector3d scale = math::Vector3d::One;
  math::Pose3d localPose = math::Pose3d::Zero;
  rendering::GeometryPtr geom{nullptr};
  if (_msg.has_box())
  {
    geom = this->scene->CreateBox();
    if (_msg.box().has_size())
      scale = msgs::Convert(_msg.box().size());
  }
  else if (_msg.has_cylinder())
  {
    geom = this->scene->CreateCylinder();
    scale.X() = _msg.cylinder().radius() * 2;
    scale.Y() = scale.X();
    scale.Z() = _msg.cylinder().length();
  }
  else if (_msg.has_capsule())
  {
    auto capsule = this->scene->CreateCapsule();
    capsule->SetRadius(_msg.capsule().radius());
    capsule->SetLength(_msg.capsule().length());
    geom = capsule;
    scale.X() = _msg.capsule().radius() * 2;
    scale.Y() = scale.X();
    scale.Z() = _msg.capsule().length() + scale.X();
  }
  else if (_msg.has_ellipsoid())
  {
    geom = this->scene->CreateSphere();
    scale.X() = _msg.ellipsoid().radii().x() * 2;
    scale.Y() = _msg.ellipsoid().radii().y() * 2;
    scale.Z() = _msg.ellipsoid().radii().z() * 2;
  }
  else if (_msg.has_plane())
  {
    geom = this->scene->CreatePlane();

    if (_msg.plane().has_size())
    {
      scale.X() = _msg.plane().size().x();
      scale.Y() = _msg.plane().size().y();
    }

    if (_msg.plane().has_normal())
    {
      // Create a rotation for the plane mesh to account for the normal vector.
      // The rotation is the angle between the +z(0,0,1) vector and the
      // normal, which are both expressed in the local (Visual) frame.
      math::Vector3d normal = msgs::Convert(_msg.plane().normal());
      localPose.Rot().From2Axes(math::Vector3d::UnitZ, normal.Normalized());
    }
  }
  else if (_msg.has_sphere())
  {
    geom = this->scene->CreateSphere();
    scale.X() = _msg.sphere().radius() * 2;
    scale.Y() = scale.X();
    scale.Z() = scale.X();
  }
  else if (_msg.has_mesh())
  {
    if (_msg.mesh().filename().empty())
    {
      return geom;
    }
    rendering::MeshDescriptor descriptor;

    // Assume absolute path to mesh file
    descriptor.meshName = _msg.mesh().filename();

    ignition::common::MeshManager* meshManager =
        ignition::common::MeshManager::Instance();
    descriptor.mesh = meshManager->Load(descriptor.meshName);
    geom = this->scene->CreateMesh(descriptor);

    scale = msgs::Convert(_msg.mesh().scale());
  }
  else
  {
    ignerr << "Unsupported geometry type" << std::endl;
  }
  _scale = scale;
  _localPose = localPose;
  return geom;
}

/////////////////////////////////////////////////
rendering::MaterialPtr SceneManager::LoadMaterial(const msgs::Material &_msg)
{
  rendering::MaterialPtr material = this->scene->CreateMaterial();
  if (_msg.has_ambient())
  {
    material->SetAmbient(msgs::Convert(_msg.ambient()));
  }
  if (_msg.has_diffuse())
  {
    material->SetDiffuse(msgs::Convert(_msg.diffuse()));
  }
  if (_msg.has_specular())
  {
    material->SetDiffuse(msgs::Convert(_msg.specular()));
  }
  if (_msg.has_emissive())
  {
    material->SetEmissive(msgs::Convert(_msg.emissive()));
  }

  return material;
}

/////////////////////////////////////////////////
rendering::LightPtr SceneManager::LoadLight(const msgs::Light &_msg)
{
  rendering::LightPtr light;

  switch (_msg.type())
  {
    case msgs::Light_LightType_POINT:
      light = this->scene->CreatePointLight();
      break;
    case msgs::Light_LightType_SPOT:
    {
      light = this->scene->CreateSpotLight();
      rendering::SpotLightPtr spotLight =
          std::dynamic_pointer_cast<rendering::SpotLight>(light);
      spotLight->SetInnerAngle(_msg.spot_inner_angle());
      spotLight->SetOuterAngle(_msg.spot_outer_angle());
      spotLight->SetFalloff(_msg.spot_falloff());
      break;
    }
    case msgs::Light_LightType_DIRECTIONAL:
    {
      light = this->scene->CreateDirectionalLight();
      rendering::DirectionalLightPtr dirLight =
          std::dynamic_pointer_cast<rendering::DirectionalLight>(light);

      if (_msg.has_direction())
        dirLight->SetDirection(msgs::Convert(_msg.direction()));
      break;
    }
    default:
      ignerr << "Light type not supported" << std::endl;
      return light;
  }

  if (_msg.has_pose())
    light->SetLocalPose(msgs::Convert(_msg.pose()));

  if (_msg.has_diffuse())
    light->SetDiffuseColor(msgs::Convert(_msg.diffuse()));

  if (_msg.has_specular())
    light->SetSpecularColor(msgs::Convert(_msg.specular()));

  light->SetAttenuationConstant(_msg.attenuation_constant());
  light->SetAttenuationLinear(_msg.attenuation_linear());
  light->SetAttenuationQuadratic(_msg.attenuation_quadratic());
  light->SetAttenuationRange(_msg.range());

  light->SetCastShadows(_msg.cast_shadows());

  this->lights[_msg.id()] = light;
  return light;
}

/////////////////////////////////////////////////
void SceneManager::DeleteEntity(const unsigned int _entity)
{
  if (this->visuals.find(_entity) != this->visuals.end())
  {
    auto visual = this->visuals[_entity].lock();
    if (visual)
    {
      this->scene->DestroyVisual(visual, true);
    }
    this->visuals.erase(_entity);
  }
  else if (this->lights.find(_entity) != this->lights.end())
  {
    auto light = this->lights[_entity].lock();
    if (light)
    {
      this->scene->DestroyLight(light, true);
    }
    this->lights.erase(_entity);
  }
}

void SceneManager::UpdatePoses(std::unordered_map<long unsigned int, math::Pose3d> &_poses)
{
  std::lock_guard<std::mutex> lock(this->mutex);
  this->poses = _poses;
}
