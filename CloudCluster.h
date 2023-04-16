#include "ParticleEmitter.hpp"


class CloudCluster
{
public:
	CloudCluster(std::vector<std::shared_ptr<EmitterController>> emittersInput, CloudType typeInput, unsigned int textureArray, unsigned int texArraySizeInput);
	
	void setupBuffers(unsigned int skySize);
	void clusterUpdate(const float dt, const double curTime, Camera camera, glm::vec3 movement);
	void drawCluster(const float dt, const double curTime, const glm::mat4 projection, const glm::mat4& view, const Camera camera, glm::vec3 movement);
	void resetCluster();

	float cameraDistance;
	glm::vec3 clusterCenter;
	float clusterRadius;
	CloudType type;
	
	bool buffersRunning;

	bool merged; 
	bool remove;

	std::vector<std::shared_ptr<EmitterController>> emitters;
	std::vector<Particle*> clusterParticles;


private:

	unsigned int textureArrayID;
	unsigned int texArraySize;

	unsigned int startingEmitterNum;
	unsigned int startingParticleNum;

	GLuint vertexBufferVBO;
	GLuint posBufferVBO;
	GLuint vertexBuffer;
	GLuint positionBuffer;

	Shader* shader;

	std::vector<float> clusterPositions;

	glm::vec3 getSpherePos();
	float RandomFloat(float a, float b);
	void updateCenter();
	void updateRadius();


};