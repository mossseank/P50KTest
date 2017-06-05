#pragma once

#include <glm\glm.hpp>
#include "vbo.hpp"


extern const size_t ParticleFormatSpecifierCount;
extern const vertex_format_specifier_t ParticleFormatSpecifier[4];


using vec2f = glm::vec2;


#pragma pack(push, 1)
struct Particle
{
public:
	float mass;
	union
	{
		struct
		{
			float x;
			float y;
		};
		vec2f pos;
	};
	union
	{
		struct
		{
			float vx;
			float vy;
		};
		vec2f vel;
	};
	union
	{
		struct
		{
			float ax;
			float ay;
		};
		vec2f acc;
	};

public:
	Particle() :
		mass{0},
		pos{0, 0},
		vel{0, 0},
		acc{0, 0}
	{ 
		
	}
};
#pragma pack(pop)