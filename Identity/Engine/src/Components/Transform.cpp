#include <stdafx.h>
#include <Components/Transform.h>
#include <Systems/TransformSystem.h>

Engine::Components::Transform::Transform(Objects::GameObject* p_gameObject) : IComponent{ p_gameObject },
    m_position{ Vector3F::zero }, m_forward{ Vector3F::forward }, m_right{ Vector3F::right },
    m_up{ Vector3F::up }, m_scale{ Vector3F::one }, m_rotation{ Quaternion{0.0, 0.0, 0.0, 1.0} } {}

Engine::Components::Transform::Transform(Objects::GameObject* p_gameObject, const Transform& p_other) : IComponent{ p_gameObject },
    m_parent{ p_other.m_parent }, m_position{ p_other.m_position }, m_forward{ p_other.m_forward }, m_right{ p_other.m_right },
    m_up{ p_other.m_up }, m_scale{ p_other.m_scale }, m_rotation{ p_other.m_rotation } {}

Engine::Components::Transform::Transform() : IComponent{nullptr},
    m_position{ Vector3F::zero }, m_forward{ Vector3F::forward }, m_right{ Vector3F::right },
    m_up{ Vector3F::up }, m_scale{ Vector3F::one }, m_rotation{ Quaternion{0.0, 0.0, 0.0, 1.0} } {}

bool Engine::Components::Transform::operator==(IComponent* p_other)
{
    return true;
}

bool Engine::Components::Transform::DeleteFromMemory()
{
    return true;
}

void Engine::Components::Transform::Translate(const Vector3F& p_vector)
{
    m_position += p_vector;
    needUpdate = true;
    // UpdateWorldTransformMatrix();
}

void Engine::Components::Transform::RotateWithEulerAngles(const Vector3F& p_euler)
{
    //TODO: check if it properly accepts angles > 360
    Quaternion quat;
    quat.MakeFromEuler(Vector3F{p_euler.x, p_euler.y, p_euler.z});
    m_rotation *= quat;
    needAxesUpdate = true;
    needUpdate = true;
    // CalculateAxes();
    // UpdateWorldTransformMatrix();
}

void Engine::Components::Transform::Scale(const Vector3F& p_scale)
{
    m_scale *= p_scale;
    needUpdate = true;
    // UpdateWorldTransformMatrix();
}

void Engine::Components::Transform::UpdateWorldTransformMatrix()
{
    m_worldTransform = Matrix4F::CreateTransformation(m_position,
                                                          m_rotation,
                                                          m_scale);
    needUpdate = false;
}

Vector3F Engine::Components::Transform::GetEuler() const
{
    return m_rotation.ToEuler();
}

std::shared_ptr<Engine::Components::Transform> Engine::Components::Transform::GetParent() const
{
    return Containers::TransformSystem::FindTransform(m_parent);
}

void Engine::Components::Transform::CalculateAxes()
{
    Quaternion quatf = (m_rotation * Vector3F::forward * m_rotation.Conjugate());
    Quaternion quatr = (m_rotation * Vector3F::right * m_rotation.Conjugate());
    Quaternion quatu = (m_rotation * Vector3F::up * m_rotation.Conjugate());
    Vector3F vec3f = quatf.GetRotationAxis();
    Vector3F vec3r = quatr.GetRotationAxis();
    Vector3F vec3u = quatu.GetRotationAxis();

    m_forward = Vector3F{ -vec3f.x,-vec3f.y, vec3f.z };
    m_right = Vector3F{ vec3r.x,vec3r.y, vec3r.z };
    m_up = Vector3F{ vec3u.x,vec3u.y, -vec3u.z };

    needAxesUpdate = false;
    // UpdateWorldTransformMatrix();
}
