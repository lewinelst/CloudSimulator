#version 460 core

in float AliveTime;
in vec2 TexCoords;
in vec4 Position;

uniform sampler2DArray textureArray;
uniform vec3 clusterCenter;
uniform int texArraySize; // amount of textures in array
uniform float clusterRadiusVal; // size of radius


uniform float yMin;
uniform float yMax;
uniform float furthestXZDist;
uniform float shadowPercentage; // how much of the particels color will be determined by the shadow (between 0.0 and 1.0)
uniform int cloudType; 

out vec4 FragColor;

// calculate texture on gpu, need cluster center and number of textures in texture array (just calculate here) same calcualtion as one done in particle emitter class 

void main()
{
	int textureNum = texArraySize - 1; // default is most transparent texture (used for cirrus)
	if (cloudType == 2) // Stratus
	{
		float centerDistance = distance(vec3(0.0, Position.y, 0.0), vec3(0.0, clusterCenter.y, 0.0)); // gets y axis distance
		for(int i = 1; i < texArraySize + 1; i++)
		{
			if (centerDistance <= ((float(i) / float(texArraySize)) * clusterRadiusVal)) {
				textureNum = i - 1;
				break;
			}
			else if (i == texArraySize)
			{
				textureNum = i - 1;
				break;
			}
		}
	}
	else if (cloudType == 1) // Cumulus
	{
		int texArraySizeUpdated = texArraySize - 1; // used to remove last texture from texture pool for cumulus. Replace all instances of texArraySizeUpdated with texArraySize to remove.
		float centerDistance = distance(Position.xyz, clusterCenter);
		for(int i = 1; i < texArraySizeUpdated + 1; i++)
		{
			if (centerDistance <= ((float(i) / float(texArraySizeUpdated)) * clusterRadiusVal)) {
				textureNum = i - 1;
				break;
			}
			else if (i == texArraySizeUpdated)
			{
				textureNum = i - 1;
				break;
			}
		}
	}

	vec3 textureCoordinate = vec3(TexCoords.x, TexCoords.y, textureNum); // was TextureNum
	vec4 texColor = texture(textureArray, textureCoordinate);
	if(texColor.a < 0.05) // smooths edges of texture and stops boxy look
		discard;

	// shading methods
	//texColor.xyz = (texColor.xyz * (1 - shadowPercentage)) + (shadowPercentage * (((Position.y - yMin) / (yMax - yMin)))); // linear
	//texColor.xyz = (texColor.xyz * (1 - shadowPercentage)) + (shadowPercentage * (exp((Position.y - yMin) / (yMax - yMin)) - 1)); // e^x + 1
	//texColor.xyz = (texColor.xyz * (1 - shadowPercentage)) + (shadowPercentage * (tanh( 2 * (Position.y - yMin) / (yMax - yMin)))); // tanh(2x)

	if (cloudType == 2)
	{
		// y shading
		texColor.xyz = (texColor.xyz * (1 - shadowPercentage)) + (shadowPercentage * (tanh( 2 * (Position.y - yMin) / (yMax - yMin)))); // tanh(2x)
		
		// xz shading 
		//float xzDistance = distance(Position.xz, clusterCenter.xz);
		//texColor.xyz = (texColor.xyz * (1 - shadowPercentage)) + (shadowPercentage * (tanh( 2 * (xzDistance) / (furthestXZDist)))); // tanh(2x)
	}
	else 
	{
		texColor.xyz = (texColor.xyz * (1 - shadowPercentage)) + (shadowPercentage * (exp((Position.y - yMin) / (yMax - yMin)) - 1)); // e^x + 1
	}


    FragColor = texColor;
};