#pragma once
#include "Core.hxx"

namespace Ngine
{
    class GameObject3D
    {
        friend class GraphicsCore;
    public:
        GameObject3D(uint32_t model, uint32_t shader);
        ~GameObject3D() = default;
        
		void SetScale(glm::vec3& scale);
		void AdjustScale(glm::vec3& scale);
		void SetRotation(glm::vec3& rotation);
		void AdjustRotation(glm::vec3& rotation);
		void SetTranslation(glm::vec3& translation);
		void AdjustTranslation(glm::vec3& translation);

		inline glm::mat4 GetWorldMatrix() const noexcept { return mWorld; }

    private:
		void RecalculateWorld();

	private:
		uint32_t mAssocMdl; //Associated model with game object
		uint32_t mAssocShader; //Associated shader with game object
		glm::vec3 mRotation = glm::vec3(0,0,0); //Rotation of game object
		glm::vec3 mTranslation = glm::vec3(0,0,0); //Translation of game object
		glm::vec3 mScale = glm::vec3(1,1,1); //Scale of game object
		glm::mat4 mWorld = glm::mat4(1.0f); //World data for MVP
    };
}