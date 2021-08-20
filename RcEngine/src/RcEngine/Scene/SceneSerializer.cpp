//
// Created by Tristan Zippert on 7/9/21.
//


#include "rcpch.h"
#include "SceneSerializer.h"
#include "Entity.h"
#include "Component.h"


#include <yaml-cpp/yaml.h>
#include <fstream>

namespace YAML {

    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

}
namespace RcEngine{
    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v){
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }
    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v){
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }


    SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
        :m_Scene(scene){
    }
    template<typename T>
    static void YamlConvertComp(YAML::Emitter& out,const char* name, Entity entity,
                                std::string value){
        auto& component = entity.GetComponent<T>();

        if(entity.template HasComponent<T>()){
            out << YAML::Key << typeid(T).name();
            std::cout<< typeid(T).name()<< std::endl;
            out << YAML::BeginMap;

            out << YAML::Key << name << YAML::Value << value;

            out <<YAML::EndMap;

        }
        out << YAML::EndMap;
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity1){
        out << YAML::BeginMap;
        out << YAML::Key << "Entity";
        out << YAML::Value << "23124232"; // Entity ID

        if(entity1.HasComponent<TagComponent>()){
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap; // TagComponent

            auto& tag = entity1.GetComponent<TagComponent>().Tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;

            out <<YAML::EndMap; // TagComponent Map
        }
        if(entity1.HasComponent<TransformComponent>()){
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap; // TagComponent

            auto& tc = entity1.GetComponent<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
            out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

            out <<YAML::EndMap; // TagComponent Map
        }
        if(entity1.HasComponent<CameraComponent>()){
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap; // TagComponent

            auto& cameraComp = entity1.GetComponent<CameraComponent>();
            auto& camera = cameraComp.Camera;

            out << YAML::Key << "Camera";
            out <<YAML::BeginMap; // Camera

            out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
            out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFov();
            out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
            out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
            out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
            out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthoFarClip();
            out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthoNearClip();
            out <<YAML::EndMap; // Camera

            out << YAML::Key << "Primary" << YAML::Value << cameraComp.Primary;
            out << YAML::Key << "FixedAspect" << YAML::Value << cameraComp.FixedAspectRatio;

            out <<YAML::EndMap; // CameraComp

        }

        if(entity1.HasComponent<SpriteRendererComponent>()){
            out <<YAML::Key << "SpriteRendererComp";
            out <<YAML::BeginMap;

            auto& spriteRenderer = entity1.GetComponent<SpriteRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << spriteRenderer.Color;
            if(spriteRenderer.Texture)
                out << YAML::Key << "TextureFile" << YAML::Value << spriteRenderer.Texture->getPath();
            out << YAML::EndMap;
        }



        out << YAML::EndMap; // Entity Map



    }
    void SceneSerializer::Serialize(const std::string &filepath) {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene";
        out << YAML::Value << "Untitled Scene";
        out << YAML::Key << "Entities";
        out << YAML::Key << YAML::BeginSeq;
        m_Scene->m_Registry.each([&](auto entityID){
            Entity entity = {entityID, m_Scene.get()};
            if(!entity)
                return;

            SerializeEntity(out, entity);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;
        std::ofstream fout(filepath);
        fout << out.c_str();
    }
    bool SceneSerializer::SerializeRuntime(const std::string &filepath) {
        return false;
    }
    bool SceneSerializer::DeSerialize(const std::string &filepath) {
        std::ifstream stream(filepath);
        std::stringstream  strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if(!data["Scene"])
            return false;

        auto sceneName = data["Scene"].as<std::string>();
        RC_CORE_TRACE("Deserializing Scene {0}",sceneName);

        auto entities = data["Entities"];
        if(entities){
            for(auto entity: entities){
                uint64_t uuid = entity["Entity"].as<uint64_t>();

                std::string name;
                auto tagComp = entity["TagComponent"];
                if(tagComp)
                    name = tagComp["Tag"].as<std::string>();

                RC_CORE_TRACE("Deserialized entity with ID ={0}, name= {1}", uuid, name);

                Entity deserializedEntity = m_Scene->CreateEntity(name);

                auto transformComponent = entity["TransformComponent"];
                if (transformComponent){
                    auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                    tc.Translation = transformComponent["Translation"].as<glm::vec3>();
                    tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    tc.Scale = transformComponent["Scale"].as<glm::vec3>();

                }

                auto cameraComponent = entity["CameraComponent"];
                if (cameraComponent)
                {
                    auto& cc = deserializedEntity.AddComponent<CameraComponent>();

                    auto cameraProps = cameraComponent["Camera"];
                    cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

                    cc.Camera.SetPerspectiveVerticalFov(cameraProps["PerspectiveFOV"].as<float>());
                    cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
                    cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

                    cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
                    cc.Camera.SetOrthoNearClip(cameraProps["OrthographicNear"].as<float>());
                    cc.Camera.SetOrthoFarClip(cameraProps["OrthographicFar"].as<float>());

                    cc.Primary = cameraComponent["Primary"].as<bool>();
                    cc.FixedAspectRatio = cameraComponent["FixedAspect"].as<bool>();
                }

                auto spriteRendererComponent = entity["SpriteRendererComp"];
                if (spriteRendererComponent)
                {
                    auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
                    src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                    auto texturein = spriteRendererComponent["TextureFile"].as<std::string>();
                    // we need to check if the file exists and cant rely on catching the error
                    if(std::filesystem::exists(texturein))
                        src.Texture = Texture2D::Create(texturein.c_str());
                }

            }
        }
        return true;
    }
    bool SceneSerializer::DeSerializeRuntime(const std::string &filepath) {
        RC_CORE_ASSERT("",false);
        return false;
    }
}