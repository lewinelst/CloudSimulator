#include "CloudCluster.h"

struct VectorField
{
	std::vector<std::vector<std::vector<glm::vec3>>> vectorField;
	int sizeX;
	int sizeY;
	int sizeZ;
};

class Cloudscape
{
public:
	Cloudscape(int sizeXInput, int sizeYInput, int sizeZInput, int stratusNumInput, int cumulusNumInput, int cirrusNumInput, unsigned int textureArray, int numOfTextures, std::vector<std::vector<std::vector<glm::vec3>>> vectorFieldIn);

	void drawScape(const float dt, const double curTime, const glm::mat4 projection, const glm::mat4& view, const Camera camera);


private:
	
	void addCloud(CloudType type, bool respawn);

	void checkForMerge();

	glm::vec3 getCloudPos(CloudType type, bool respawn);
	float RandomFloat(float a, float b);

	int sizeX;
	int sizeY;
	int sizeZ;

	int stratusNum;
	int cumulusNum;
	int cirrusNum;

	unsigned int textureArray;
	int numOfTextures;

	unsigned int particleNum;
	
	std::vector<std::shared_ptr<CloudCluster>> clouds;

	VectorField vc;

};