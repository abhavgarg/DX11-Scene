#pragma once
class Ball
{
public:
	Ball();
	~Ball();

	void							Update();
	DirectX::SimpleMath::Matrix		getBallMatrix();
	void							setPosition(DirectX::SimpleMath::Vector3 newPosition);
	DirectX::SimpleMath::Vector3	getPosition();
	DirectX::SimpleMath::Vector3	getForward();
	DirectX::SimpleMath::Vector3	getRight();
	void							setRotation(DirectX::SimpleMath::Vector3 newRotation);
	DirectX::SimpleMath::Vector3	getRotation();
	float							getMoveSpeed();
	float							getRotationSpeed();

private:
	DirectX::SimpleMath::Matrix		m_ballMatrix;			//ball matrix to be passed out and used to set camera position and angle for wrestling
	DirectX::SimpleMath::Vector3	m_lookat;
	DirectX::SimpleMath::Vector3	m_position;
	DirectX::SimpleMath::Vector3	m_forward;
	DirectX::SimpleMath::Vector3	m_right;
	DirectX::SimpleMath::Vector3	m_up;
	DirectX::SimpleMath::Vector3	m_orientation;			//vector storing pitch yaw and roll. 

	float	m_movespeed;
	float	m_ballRotRate;

	float xmin;
	float xmax;
	float zmin;
	float zmax;
	float radius;

};
#pragma once
