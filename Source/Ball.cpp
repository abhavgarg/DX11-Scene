#include "pch.h"
#include "Ball.h"

//camera for our app simple directX application. While it performs some basic functionality its incomplete. 
//

Ball::Ball()
{
	//initalise values. 
	//Orientation and Position are how we control the camera. 
	m_orientation.x = 90.0f;		//rotation around x - pitch
	m_orientation.y = 0.0f;		//rotation around y - yaw
	m_orientation.z = 0.0f;		//rotation around z - roll	//we tend to not use roll a lot in first person

	m_position.x = 0.0f;		//camera position in space. 
	m_position.y = -0.8f;
	m_position.z = 0.0f;

	//These variables are used for internal calculations and not set.  but we may want to queary what they 
	//externally at points
	m_lookat.x = 0.0f;		//Look target point
	m_lookat.y = 0.0f;
	m_lookat.z = 0.0f;

	m_forward.x = 0.0f;		//forward/look direction
	m_forward.y = 0.0f;
	m_forward.z = 0.0f;

	m_right.x = 0.0f;
	m_right.y = 0.0f;
	m_right.z = 0.0f;

	//
	m_movespeed = 0.30;
	m_ballRotRate = 3.0;
	radius = 0.075;

	//force update with initial values to generate other camera data correctly for first update. 
	Update();
}


Ball::~Ball()
{
}

void Ball::Update()
{
	//rotation in yaw - using the paramateric equation of a circle
	m_forward.x = sin((m_orientation.y)*3.1415f / 180.0f);
	m_forward.z = cos((m_orientation.y)*3.1415f / 180.0f);
	m_forward.Normalize();

	//m_right.y = sin((m_orientation.x)*3.1415f / 180.0f);
	//m_right.z = cos((m_orientation.x)*3.1415f / 180.0f);
	//m_right.Normalize();


	//create right vector from look Direction
	m_forward.Cross(DirectX::SimpleMath::Vector3::UnitY, m_right);

	//update lookat point
	m_lookat = m_position + m_forward;

	//apply camera vectors and create camera matrix
	m_ballMatrix = (DirectX::SimpleMath::Matrix::CreateLookAt(m_position, m_lookat, DirectX::SimpleMath::Vector3::UnitY));

	xmin = m_position.x - radius;
	xmax = m_position.x + radius;
	zmin = m_position.z - radius;
	zmax = m_position.z + radius;
}

DirectX::SimpleMath::Matrix Ball::getBallMatrix()
{
	return m_ballMatrix;
}

void Ball::setPosition(DirectX::SimpleMath::Vector3 newPosition)
{
	m_position = newPosition;
}

DirectX::SimpleMath::Vector3 Ball::getPosition()
{
	return m_position;
}

DirectX::SimpleMath::Vector3 Ball::getForward()
{
	return m_forward;
}

DirectX::SimpleMath::Vector3 Ball::getRight()
{
	return m_right;
}

void Ball::setRotation(DirectX::SimpleMath::Vector3 newRotation)
{
	m_orientation = newRotation;
}

DirectX::SimpleMath::Vector3 Ball::getRotation()
{
	return m_orientation;
}

float Ball::getMoveSpeed()
{
	return m_movespeed;
}

float Ball::getRotationSpeed()
{
	return m_ballRotRate;
}
