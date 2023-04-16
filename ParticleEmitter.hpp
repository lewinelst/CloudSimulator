#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "shader.h"
#include "stb_image.h"
#include "camera.h"

#include <string>
#include <vector>
#include <random>
#include <functional>
#include <iostream>
#include <optional>

enum State
{
	constant,
	billowing,
	still
};

enum CloudType
{
	Stratus,
	Cumulus,
	Cirrus
};

struct Particle
{
	glm::vec3 position;
	glm::vec3 startingPos;
	float lifetime;
	float aliveTime;
	float textureNum;
	float cameraDistance;


	bool operator<(Particle& comparisonParticle) { // comparison function used for sorting
		// sorts in reverse order
		return this->cameraDistance > comparisonParticle.cameraDistance;
	}
};

// emitter
class ParticleEmitter
{
public:

	ParticleEmitter(glm::vec3 emitterPos);
	~ParticleEmitter();

	void update( const float dt, const double curTime, Camera camera, std::optional<CloudType> clusterType, glm::vec3 movement);
	void reset();

	glm::vec3 emitterPos;
	float cameraDistance;

	std::vector< Particle > particles;

private:
	float lowerLife;
	float upperLife;
	float radius; 

	glm::vec3 getSpherePos(std::optional<CloudType> clusterType);

	float RandomFloat(float a, float b);
};



// controller
class EmitterController
{
public:
	EmitterController(glm::vec3 emitterPos);
	~EmitterController();

	ParticleEmitter* emitter;

private:
	unsigned int* textureArrayID;
	
};