#include "GameObject.h"

namespace Ngine {
    GameObject3D::GameObject3D(uint32_t model, uint32_t shader)
    {
        mAssocShader = shader;
        mAssocMdl = model;

        RecalculateWorld();
    }

    void GameObject3D::SetScale(glm::vec3& scale)
    {
        mScale = scale;
        RecalculateWorld();
    }

    void GameObject3D::AdjustScale(glm::vec3& scale)
    {
        mScale += scale;
        RecalculateWorld();
    }

    void GameObject3D::SetRotation(glm::vec3& rotation)
    {
        mRotation = rotation;
	    RecalculateWorld();
    }

    void GameObject3D::AdjustRotation(glm::vec3& rotation)
    {
        mRotation += rotation;
	    RecalculateWorld();
    }

    void GameObject3D::SetTranslation(glm::vec3& translation)
    {
        mTranslation = translation;
	    RecalculateWorld();
    }

    void GameObject3D::AdjustTranslation(glm::vec3& translation)
    {
        mTranslation += translation;
	    RecalculateWorld();
    }

    void GameObject3D::RecalculateWorld()
    {
        mWorld = glm::mat4(1.0f);

        //Rotate model
        mWorld = glm::rotate(mWorld, glm::radians(mRotation.x), glm::vec3(1,0,0));
        mWorld = glm::rotate(mWorld, glm::radians(mRotation.y), glm::vec3(0,1,0));
        mWorld = glm::rotate(mWorld, glm::radians(mRotation.z), glm::vec3(0,0,1));
        //Translate model
        mWorld = glm::translate(mWorld, mTranslation);
        //Scale model
        mWorld = glm::scale(mWorld, mScale);
    }
}