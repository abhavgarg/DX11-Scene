//
// Game.cpp
//

#include "pch.h"
#include "Game.h"


//toreorganise
#include <fstream>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{

    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }

}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{

	m_input.Initialise(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	//setup light
	m_Light.setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	m_Light.setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light.setPosition(0.0f, 5.0f, 1.0f);
	m_Light.setDirection(-1.0f, -1.0f, 0.0f);
	//setup camera
	m_Camera01.setPosition(Vector3(-2.0f, 0.5f, 0.0f));
	m_Camera01.setRotation(Vector3(-90.0f, 90.0f, 0.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up.

	//setup Ball
	m_Ball.setPosition(Vector3(0.0f, 1.0f, 0.0f));
	m_Ball.setRotation(Vector3(0.0f, 0.0f, 0.0f));
	m_radius = 0.075f;

    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    //m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"crowd.wav");
    m_effect1 = m_soundEffect->CreateInstance();
   // m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    //m_effect2->Play();

}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	//take in input
	m_input.Update();								//update the hardware
	m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game
	
	//Update all game objects
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

	//Render all game content. 
    Render();


    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }

	
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{

	Vector3 position = m_Camera01.getPosition();
	Vector3 Ballpos = m_Ball.getPosition();
	//note that currently.  Delta-time is not considered in the game object movement. 
	if (m_gameInputCommands.rotLeft)
	{
		Vector3 rotation = m_Camera01.getRotation();
		rotation.y = rotation.y += m_Camera01.getRotationSpeed()*m_timer.GetElapsedSeconds();
		m_Camera01.setRotation(rotation);
	}
	if (m_gameInputCommands.rotRight)
	{
		Vector3 rotation = m_Camera01.getRotation();
		rotation.y = rotation.y -= m_Camera01.getRotationSpeed()*m_timer.GetElapsedSeconds();
		m_Camera01.setRotation(rotation);
	}

	if (position.x >= -5.5f && position.x <= 5.5f && position.z <= 4.0f && position.z >= -4.0f)
	{

		if (m_gameInputCommands.forward)
		{
			position += (m_Camera01.getForward()*m_Camera01.getMoveSpeed()*m_timer.GetElapsedSeconds()); //add the forward vector	
		}
		if (m_gameInputCommands.back)
		{
			position -= (m_Camera01.getForward()*m_Camera01.getMoveSpeed()*m_timer.GetElapsedSeconds()); //add the forward vector
		}
		if (m_gameInputCommands.left)
		{
			position -= (m_Camera01.getRight()*m_Camera01.getMoveSpeed()*m_timer.GetElapsedSeconds()); //add the forward vector
		}
		if (m_gameInputCommands.right)
		{
			position += (m_Camera01.getRight()*m_Camera01.getMoveSpeed()*m_timer.GetElapsedSeconds()); //add the forward vector
		}
		m_Camera01.setPosition(position);
	}
	else //keep camera in bounds
	{
		if (position.x >= 5.5f)
		{
			position.x = 5.5f;			
		}

		else if (position.x <= -5.5f)
		{
			position.x = -5.5f;
		}

		else if (position.z >= 4.0f)
		{
			position.z = 4.0f;
		}

		else if (position.z <= 4.0f)
		{
			position.z = -4.0f;
		}
		m_Camera01.setPosition(position);
	} 
	
	//check collision
	if (Ballpos.x - position.x < 0.175f || Ballpos.z - position.z < 0.175f ||  position.x -Ballpos.x < 0.175f || position.z - Ballpos.z < 0.175f) //0.175 = radius of ball + radius of camera
	{
		Ballpos += (m_Ball.getForward()*m_Ball.getMoveSpeed());
		m_Ball.setPosition(Ballpos);
	}

	m_Camera01.Update();	//camera update.
	m_view = m_Camera01.getCameraMatrix();
	m_world = Matrix::Identity;


    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }


  
	if (m_input.Quit())
	{
		ExitGame();
	}
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{	
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
	float delta = float(m_timer.GetFramesPerSecond());//getting the fps value from m_timer and storing it in delta
    // Draw Text to the screen


    m_deviceResources->PIXBeginEvent(L"Draw sprite");
    m_sprites->Begin();

	std::string fpsText = std::to_string(delta);
	fpsText = "FPS=" + fpsText;
	m_font->DrawString(m_sprites.get(),fpsText.c_str(), XMFLOAT2(10, 10), Colors::Black);

	//m_font->DrawString(m_sprites.get(), L"Football Field", XMFLOAT2(10, 10), Colors::Black);
    m_sprites->End();
    m_deviceResources->PIXEndEvent();
	
	//Set Rendering states. 
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullClockwise());
//	context->RSSetState(m_states->Wireframe());


	//prepare transform for ball.

	//SimpleMath::Matrix newPositionx = SimpleMath::Matrix::CreateTranslation(0.0f, 1.0f, 0.0f);
	SimpleMath::Matrix newPositionx = m_Ball.getBallMatrix();
	SimpleMath::Matrix newScalex = SimpleMath::Matrix::CreateScale(0.15f, 0.15f, 0.15f);//radius=0.075
	m_world = m_world * newPositionx *newScalex;

	//setup and draw ball
	// Turn our shaders on,  set parameters
	m_BasicShaderPair.EnableShader(context);
	m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture3.Get());

	//render our model
	m_BasicModel.Render(context);

	//prepare transform for 1st goalpost. 
	m_world = SimpleMath::Matrix::Identity; //set world back to identity
	SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(113.0f, 0.0f, -3.2f);
	SimpleMath::Matrix newScale2 = SimpleMath::Matrix::CreateScale(0.05f, 0.05f, 0.045f);
	m_world = m_world * newPosition *newScale2;

	//setup and draw goalpost
	m_BasicShaderPair.EnableShader(context);
	m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture2.Get());
	m_BasicModel2.Render(context);

	//prepare transform for floor object. 
	m_world = SimpleMath::Matrix::Identity; //set world back to identity
	SimpleMath::Matrix newPosition2 = SimpleMath::Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);
	m_world = m_world * newPosition2;

	//setup and draw cube
	m_BasicShaderPair.EnableShader(context);
	m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture1.Get());
	m_BasicModel3.Render(context);

	//prepare transform for 2nd goalpost. 
	SimpleMath::Matrix newPosition4 = SimpleMath::Matrix::CreateTranslation(-113.0f, 0.0f, -3.2f);
	SimpleMath::Matrix newScale4 = SimpleMath::Matrix::CreateScale(0.05f, 0.05f, 0.045f);
	m_world = m_world * newPosition4 *newScale4;

	//setup and draw goalpost
	m_BasicShaderPair.EnableShader(context);
	m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture2.Get());
	m_BasicModel4.Render(context);


    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{

    m_audEngine->Suspend();

}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();


    m_audEngine->Resume();

}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}


// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	//setup our test model
	m_BasicModel.InitializeSphere(device);

	m_BasicModel2.InitializeModel(device,"goalpost.obj");
	m_BasicModel3.InitializeBox(device, 13.0f, 0.1f, 9.0f);	//box includes dimensions
	m_BasicModel4.InitializeModel(device, "goalpost.obj");
	//load and set up our Vertex and Pixel Shaders
	m_BasicShaderPair.InitStandard(device, L"light_vs.cso", L"light_ps.cso");

	//load Textures
	CreateDDSTextureFromFile(device, L"fields.dds",		nullptr,	m_texture1.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"goalpost.dds", nullptr,	m_texture2.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"Ball_tex.dds", nullptr, m_texture3.ReleaseAndGetAddressOf());
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        100.0f
    );
}


void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
	m_batch.reset();
	m_testmodel.reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
