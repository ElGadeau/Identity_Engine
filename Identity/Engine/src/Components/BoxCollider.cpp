#include <stdafx.h>

#include <LinearMath/btVector3.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>

#include <Components/BoxCollider.h>
#include <Objects/GameObject.h>
#include <Systems/PhysicsSystem.h>
#include <Managers/ResourceManager.h>

Engine::Components::BoxCollider::BoxCollider(Objects::GameObject* p_gameObject) : IComponent{p_gameObject, BOX_COLLIDER}
{
    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    m_box = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    m_dimensions = { 1.0f, 1.0f, 1.0f };
    btTransform trans;
    auto       position = m_gameObject->GetTransform()->GetPosition();
    auto       rotation = m_gameObject->GetTransform()->GetRotation();

    trans.setIdentity();

    trans.setOrigin(btVector3(position.x, position.y, position.z));
    trans.setRotation(btQuaternion(rotation.GetXAxisValue(), rotation.GetYAxisValue(), rotation.GetZAxisValue(), rotation.w));

    m_box->calculateLocalInertia(m_mass, localInertia);

    m_motionState = new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(m_mass, m_motionState, m_box, localInertia);
    m_rigidbody = new btRigidBody(rbInfo);

    ObjectElements::Model model = ConstructBox();

    const int32_t id = Managers::ResourceManager::AddModel(model);
    m_model = Managers::ResourceManager::FindModel(id);

    Systems::PhysicsSystem::AddBoxCollider(this);
}

Engine::Components::BoxCollider::BoxCollider(Objects::GameObject* p_gameObject, std::vector<std::string> p_block) : IComponent{ p_gameObject, BOX_COLLIDER }
{
    std::vector<std::string> words;

    for (int i = 0; i < p_block.size(); ++i)
    {
        std::stringstream stringStream(p_block[i]);

        do
        {
            std::string word;
            stringStream >> word;
            words.push_back(word);
        } while (stringStream);

        if (words[0] == "m_mass")
        {
            m_mass = std::stof(words[1]);
        }
        else if (words[0] == "m_offset")
        {
            m_offset.x = std::stof(words[1]);
            m_offset.y = std::stof(words[2]);
            m_offset.z = std::stof(words[3]);
        }
        else if (words[0] == "m_box")
        {
            m_box = new btBoxShape(btVector3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3])));
        }

        words.clear();
    }

    //TODO: Memory leak here
    /*delete m_box;
    delete m_rigidbody;
    delete m_motionState;*/

    // m_box->getHalfExtentsWithMargin().getX()
    // m_box->getHalfExtentsWithMargin().getY()
    // m_box->getHalfExtentsWithMargin().getZ()
    
    m_dimensions = { m_box->getHalfExtentsWithMargin().getX(),
                     m_box->getHalfExtentsWithMargin().getY(),
                     m_box->getHalfExtentsWithMargin().getZ() };

    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    btTransform trans;
    auto position = m_gameObject->GetTransform()->GetPosition();
    auto rotation = m_gameObject->GetTransform()->GetRotation();

    Vector3D offsetD = m_offset;
    Quaternion q = rotation * offsetD * Quaternion::Conjugate(rotation);
    offsetD = { q.GetXAxisValue(), q.GetYAxisValue(), q.GetZAxisValue() };
    Vector3D offset = offsetD;
    position += offset;

    trans.setIdentity();
    trans.setOrigin(btVector3(position.x, position.y, position.z));
    trans.setRotation(btQuaternion(rotation.GetXAxisValue(), rotation.GetYAxisValue(), rotation.GetZAxisValue(), rotation.w));
    m_box->calculateLocalInertia(m_mass, localInertia);

    m_motionState = new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(m_mass, m_motionState, m_box, localInertia);
    m_rigidbody = new btRigidBody(rbInfo);

    ObjectElements::Model model = ConstructBox();

    //TODO add a "AddModel" that take a model as parametre
    const int32_t id = Managers::ResourceManager::AddModel(model);
    m_model = Managers::ResourceManager::FindModel(id);

    Systems::PhysicsSystem::AddBoxCollider(this);
}

Engine::Components::BoxCollider::BoxCollider(Objects::GameObject* p_gameObject, std::shared_ptr<BoxCollider> p_other) : IComponent{p_gameObject, BOX_COLLIDER}
{
    //init data
    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    m_mass = p_other->m_mass;
    m_offset = p_other->m_offset;
    m_box = p_other->m_box;
    m_dimensions = p_other->m_dimensions;
    // m_motionState = p_other->m_motionState;

    btTransform trans;
    // p_other->m_motionState->getWorldTransform(trans);
    auto       position = m_gameObject->GetTransform()->GetPosition();
    auto       rotation = m_gameObject->GetTransform()->GetRotation();
    
    Vector3D offsetD = m_offset;
    Quaternion q = rotation * offsetD * Quaternion::Conjugate(rotation);
    
    offsetD = { q.GetXAxisValue(), q.GetYAxisValue(), q.GetZAxisValue() };
    Vector3D offset = offsetD;
    
    // position += offset;
    
    trans.setIdentity();
    trans.setOrigin(btVector3(position.x, position.y, position.z));
    trans.setRotation(btQuaternion(rotation.GetXAxisValue(), rotation.GetYAxisValue(), rotation.GetZAxisValue(), rotation.w));

    m_box->calculateLocalInertia(m_mass, localInertia);

    m_motionState = new btDefaultMotionState(trans);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(m_mass, m_motionState, m_box, localInertia);
    m_rigidbody = new btRigidBody(rbInfo);

    ObjectElements::Model model = ConstructBox();
    //TODO add a "AddModel" that take a model as parametre
    const int32_t id = Managers::ResourceManager::AddModel(model);
    m_model = Managers::ResourceManager::FindModel(id);

    Systems::PhysicsSystem::AddBoxCollider(this);
}

Engine::Components::BoxCollider::~BoxCollider()
{
    Engine::Systems::PhysicsSystem::GetWorld()->removeRigidBody(m_rigidbody);
    delete m_box;
    delete m_motionState;
    delete m_rigidbody;
}

void Engine::Components::BoxCollider::Serialize(std::ostream& p_stream)
{
    p_stream << typeid(*this).name() << "\n{\n" <<
        "   m_mass " << m_mass << "\n" <<
        "   m_offset " << m_offset.x << " " << m_offset.y << " " << m_offset.z << "\n" <<
        "   m_box " << m_box->getHalfExtentsWithMargin().getX() << " " << m_box->getHalfExtentsWithMargin().getY() << " " << m_box->getHalfExtentsWithMargin().getZ() << "\n" <<
        "   m_model " << m_model->GetID() << "\n" <<
        "}\n";
}

void Engine::Components::BoxCollider::Deserialize(Objects::GameObject* p_gameObject, std::vector<std::string>& p_block)
{
    m_gameObject = p_gameObject;

    std::vector<std::string> words;

    for (int i = 0; i < p_block.size(); ++i)
    {
        std::stringstream stringStream(p_block[i]);

        do
        {
            std::string word;
            stringStream >> word;
            words.push_back(word);
        } while (stringStream);

        if (words[0] == "m_mass")
        {
            m_mass = std::stof(words[1]);
        }
        else if (words[0] == "m_offset")
        {
            m_offset.x = std::stof(words[1]);
            m_offset.y = std::stof(words[2]);
            m_offset.z = std::stof(words[3]);
        }
        else if (words[0] == "m_box")
        {
            m_dimensions = { std::stof(words[1]), std::stof(words[2]), std::stof(words[3]) };
            m_box = new btBoxShape(btVector3(m_dimensions.x, m_dimensions.y, m_dimensions.z));
        }

        words.clear();
    }

    //TODO: Memory leak here
    /*delete m_box;
    delete m_rigidbody;
    delete m_motionState;*/

    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    btTransform trans;
    auto position = m_gameObject->GetTransform()->GetPosition();
    auto rotation = m_gameObject->GetTransform()->GetRotation();

    Vector3D offsetD = m_offset;
    Quaternion q = rotation * offsetD * Quaternion::Conjugate(rotation);
    offsetD = { q.GetXAxisValue(), q.GetYAxisValue(), q.GetZAxisValue() };
    Vector3D offset = offsetD;
    position += offset;

    trans.setIdentity();
    trans.setOrigin(btVector3(position.x, position.y, position.z));
    trans.setRotation(btQuaternion(rotation.GetXAxisValue(), rotation.GetYAxisValue(), rotation.GetZAxisValue(), rotation.w));
    m_box->calculateLocalInertia(m_mass, localInertia);

    m_motionState = new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(m_mass, m_motionState, m_box, localInertia);
    m_rigidbody = new btRigidBody(rbInfo);

    ObjectElements::Model model = ConstructBox();

    //TODO add a "AddModel" that take a model as parametre
    const int32_t id = Managers::ResourceManager::AddModel(model);
    m_model = Managers::ResourceManager::FindModel(id);
}

Matrix4F Engine::Components::BoxCollider::GetWorldMatrix() const
{
    btScalar    m[16];
    btTransform trans;

    m_motionState->getWorldTransform(trans);
    trans.getOpenGLMatrix(m);

    Matrix4F mat
    {
        m[0], m[1], m[2], m[3],
        m[4], m[5], m[6], m[7],
        m[8], m[9], m[10], m[11],
        m[12], m[13], m[14], m[15]
    };

    return mat.Transpose();
}

void Engine::Components::BoxCollider::SetPositionOffset(GPM::Vector3F p_offset)
{
    if (this == nullptr)
        return;

    //TODO change the center of object here
    m_offset = p_offset;

    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    btTransform trans;
    auto position = m_gameObject->GetTransform()->GetPosition();
    auto rotation = m_gameObject->GetTransform()->GetRotation();

    Vector3D offsetD = m_offset;
    Quaternion q = rotation * offsetD * Quaternion::Conjugate(rotation);
    offsetD = { q.GetXAxisValue(), q.GetYAxisValue(), q.GetZAxisValue() };
    Vector3D offset = offsetD;
    position += offset;

    trans.setIdentity();
    trans.setOrigin(btVector3(position.x, position.y, position.z));
    trans.setRotation(btQuaternion(rotation.GetXAxisValue(), rotation.GetYAxisValue(), rotation.GetZAxisValue(), rotation.w));

    m_box->calculateLocalInertia(m_mass, localInertia);
    m_motionState = new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(m_mass, m_motionState, m_box, localInertia);
    m_rigidbody = new btRigidBody(rbInfo);

}

void Engine::Components::BoxCollider::SetMass(float p_mass)
{
    m_mass = p_mass;

    if (m_rigidbody)
    {
        Systems::PhysicsSystem::GetWorld()->removeRigidBody(m_rigidbody);
    }

    auto localInertia = btVector3(0.0f, 0.0f, 0.0f);

    if (m_box)
        m_box->calculateLocalInertia(m_mass, localInertia);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(m_mass, m_motionState, m_box, localInertia);
    delete m_rigidbody;
    m_rigidbody = new btRigidBody(rbInfo);
    Systems::PhysicsSystem::GetWorld()->addRigidBody(m_rigidbody);
}

void Engine::Components::BoxCollider::SetDimensions(const GPM::Vector3F& p_dimensions)
{
    if (this == nullptr)
        return;

    m_dimensions = p_dimensions;

    btVector3 localInertia(0.0f, 0.0f, 0.0f);

    if (m_rigidbody)
    {
        delete m_box;

        Systems::PhysicsSystem::GetWorld()->removeRigidBody(m_rigidbody);
        m_box = new btBoxShape(btVector3(p_dimensions.x, p_dimensions.y, p_dimensions.z));
        m_rigidbody->setCollisionShape(m_box);
        m_box->calculateLocalInertia(m_mass, localInertia);
        m_rigidbody->setMassProps(m_mass, localInertia);
        m_rigidbody->updateInertiaTensor();
        Systems::PhysicsSystem::GetWorld()->addRigidBody(m_rigidbody);
    }

    ObjectElements::Model model = ConstructBox();
    Managers::ResourceManager::RemoveModel(m_model->GetID());
    const int32_t id = Managers::ResourceManager::AddModel(model);
    m_model = Managers::ResourceManager::FindModel(id);
}

bool Engine::Components::BoxCollider::DeleteFromMemory()
{
    Managers::ResourceManager::RemoveModel(m_model->GetID());
    Systems::PhysicsSystem::RemoveCollider(GetID());
    return true;
}

void Engine::Components::BoxCollider::SetActive(bool p_active)
{
    m_isActive = p_active;

    if (!m_isActive)
    {
        m_rigidbody->forceActivationState(DISABLE_SIMULATION);
        m_rigidbody->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
    else
    {
        m_rigidbody->forceActivationState(ACTIVE_TAG);
        m_rigidbody->setCollisionFlags(!btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
}

Engine::ObjectElements::Model Engine::Components::BoxCollider::ConstructBox()
{
    btVector3   a;
    btVector3   b;
    btTransform trans;

    m_motionState->getWorldTransform(trans);
    m_box->getAabb(trans, a, b);

    std::vector<Geometry::Vertex> vertices;
    std::vector<unsigned short>   indices;

    for (int i = 0; i < m_box->getNumVertices(); ++i)
    {
        btVector3 vertex;
        m_box->getVertex(i, vertex);
        vertices.emplace_back(Geometry::Vertex{ Vector3F{vertex.getX(), vertex.getY(), vertex.getZ()}, Vector2F{}, GPM::Vector3F{} });
    }

    //Back

    indices.emplace_back(0);
    indices.emplace_back(1);
    indices.emplace_back(2);

    indices.emplace_back(2);
    indices.emplace_back(3);
    indices.emplace_back(0);


    //Left
    indices.emplace_back(4);
    indices.emplace_back(0);
    indices.emplace_back(2);

    indices.emplace_back(2);
    indices.emplace_back(6);
    indices.emplace_back(4);

    //bottom
    indices.emplace_back(0);
    indices.emplace_back(4);
    indices.emplace_back(5);

    indices.emplace_back(5);
    indices.emplace_back(1);
    indices.emplace_back(0);

    //right
    indices.emplace_back(5);
    indices.emplace_back(1);
    indices.emplace_back(3);

    indices.emplace_back(3);
    indices.emplace_back(7);
    indices.emplace_back(5);

    //top
    indices.emplace_back(7);
    indices.emplace_back(3);
    indices.emplace_back(2);

    indices.emplace_back(2);
    indices.emplace_back(6);
    indices.emplace_back(7);

    //front
    indices.emplace_back(6);
    indices.emplace_back(5);
    indices.emplace_back(4);

    indices.emplace_back(4);
    indices.emplace_back(7);
    indices.emplace_back(6);

    ObjectElements::Model model;

    auto mesh = std::make_shared<ObjectElements::Mesh>(vertices, indices);

    model.AddMesh(mesh);
    return model;
}
