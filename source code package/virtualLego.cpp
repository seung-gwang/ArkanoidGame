////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: ¹ÚÃ¢Çö Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for ARKANOID GAME 
// Modified by 20164899 ³ë½Â±¤
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>

IDirect3DDevice9* Device = NULL;

// window size
const int Width  = 1280;
const int Height = 950;

// There are BALL_NUM number of balls
#define BALL_NUM 45
//Game start signal
bool notYetStart = true;

// initialize the position (coordinate) of each ball (ball0 ~ ball BALL_NUM-1)
const float spherePos[BALL_NUM][2] = {//Position balls to draw Smiley
/*Smiley face*/
{-3.0f,0} , {+3.0f,0} , {0,3.0f} , {0,-3.0f} ,
{-2.9544f, 0.5209f}, {-2.8190f, 1.0260f}, {-2.5980f, 1.5000f} , {-2.2981f, 1.9283f}, {-1.9283f, 2.2981f}, {-1.5f, 2.5980f}, {-1.0260f, 2.8190f} , {-0.5209f, 2.9544f},
{2.9544f, 0.5209f}, {2.8190f, 1.0260f}, {2.5980f, 1.5000f} , {2.2981f, 1.9283f}, {1.9283f, 2.2981f}, {1.5f, 2.5980f}, {1.0260f, 2.8190f} , {0.5209f, 2.9544f},
{2.9544f, -0.5209f}, {2.8190f, -1.0260f}, {2.5980f, -1.5000f} , {2.2981f, -1.9283f}, {1.9283f, -2.2981f}, {1.5f, -2.5980f}, {1.0260f, -2.8190f} , {0.5209f, -2.9544f},
{-2.9544f, -0.5209f}, {-2.8190f, -1.0260f}, {-2.5980f, -1.5000f} , {-2.2981f, -1.9283f}, {-1.9283f, -2.2981f}, {-1.5f, -2.5980f}, {-1.0260f, -2.8190f} , {-0.5209f, -2.9544f},

/*Smiley eyes*/
{-1, -1.3f},{-1, 1.3f},

/*Smiley lips*/
{ 1.2980f,1.5f}, {1.5190f,1.0260f} , {1.6544f,0.5209f}, {1.5190f,-1.0260f} , {1.6544f,-0.5209f}, {1.2980f,-1.5f}, {1.7f,0}
};


// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 1 //0.9982

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private :
	float					center_x, center_y, center_z;
    float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
	bool                    hitterDesignation = false;

public:
    CSphere(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_radius = 0;
		m_velocity_x = 0;
		m_velocity_z = 0;
        m_pSphereMesh = NULL;
    }
    ~CSphere(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
        if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
            return false;
        return true;
    }
	
    void destroy(void)
    {
        if (m_pSphereMesh != NULL) {
            m_pSphereMesh->Release();
            m_pSphereMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pSphereMesh->DrawSubset(0);
    }
	
    bool hasIntersected(CSphere& ball) 
	{
		// Insert your code here.

		//calculate the distance between the center of the balls
		D3DXVECTOR3 dist = ball.getCenter() - this->getCenter();
		float centerDistance = sqrt(dist.x * dist.x + dist.z * dist.z);

		if (centerDistance <= ball.getRadius() + this->getRadius() ) {
			return true;
		}
		else{
		return false;
		}
	}
	
	void hitBy(CSphere& ball) //ball hits 'this', ball is the hitter, ball changes direction, this destroyed if it is the target
	{ 
		// Insert your code here.
		if (this->hasIntersected(ball)) {
			
			/*collision --> change velocity*/
			D3DXVECTOR3 centerDistance = ball.getCenter() - this->getCenter();
			double dx = centerDistance.x;
			double dz = centerDistance.z;

			double vx = ball.getVelocity_X();
			double vz = ball.getVelocity_Z();
			
			double dVal = sqrt(dx * dx + dz * dz);
			double vVal = sqrt(vx * vx + vz * vz);
			double factor = vVal / dVal;


			ball.setPower(dx * factor, dz * factor);

			/*as far as it is the target ball destroy the ball by hiding it behind the plane*/
			if (!this->hitterCheck()) {
				this->setCenter(-5.0f, -5.0f, -5.0f);
			}
		}
	}

	void ballUpdate(float timeDiff) 
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		
		if(vx > 0.01 || vz > 0.01)
		{
			
			float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
			float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;
		
			this->setCenter(tX, cord.y, tZ);
		}
		else { this->setPower(0,0);}
		
	}

	double getVelocity_X() { return this->m_velocity_x;	}
	double getVelocity_Z() { return this->m_velocity_z; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x=x;	center_y=y;	center_z=z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}
	
	float getRadius(void)  const { return (float)(M_RADIUS);  }
    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
    D3DXVECTOR3 getCenter(void) const
    {
        D3DXVECTOR3 org(center_x, center_y, center_z);
        return org;
    }

	void designateHitter() { hitterDesignation = true; }
	bool hitterCheck() { return hitterDesignation; }
	
private:
    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pSphereMesh;
	
};



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:

	float					m_x;
	float					m_z;
	float                   m_width;
	float                   m_depth;
	float					m_height;

public:
	CWall(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_width = 0;
		m_depth = 0;
		m_pBoundMesh = NULL;
	}
	~CWall(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		m_width = iwidth;
		m_depth = idepth;

		if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
			return false;
		return true;
	}
	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release();
			m_pBoundMesh = NULL;
		}
	}
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
	}

	bool hasIntersected(CSphere& ball) {
		// Insert your code here.
		float ballPosX = ball.getCenter().x;
		float ballPosZ = ball.getCenter().z;
		float wallPosX = this->getPosition().x;
		float wallPosZ = this->getPosition().z;
		float wallVertical = this->getWidth();
		float wallHorizontal = this->getDepth();
		float ballRadius = ball.getRadius();

		//each represents the boundary with additional margin of Radius
		float wallUp = wallPosX - wallVertical * 0.5 - ballRadius;
		float wallLow = wallPosX + wallVertical * 0.5 + ballRadius;
		float wallLeft = wallPosZ - wallHorizontal * 0.5 - ballRadius;
		float wallRight = wallPosZ + wallHorizontal * 0.5 + ballRadius;

		//x-axis value increases downward. Therefore wallLow > wallUp
		if ( (wallUp <= ballPosX && ballPosX <= wallLow) && (wallLeft <= ballPosZ && ballPosZ <= wallRight) ) {
			return true;
		}
		else {
			return false;
		}
	}

	void hitBy(CSphere& ball) {
		// Insert your code here.

		if (this->hasIntersected(ball)) {
			double vx = ball.getVelocity_X();
			double vz = ball.getVelocity_Z();

			float ballPosX = ball.getCenter().x;
			float ballPosY = ball.getCenter().y;
			float ballPosZ = ball.getCenter().z;

			float wallPosX = this->getPosition().x;
			float wallPosZ = this->getPosition().z;
			float wallVertical = this->getWidth();
			float wallHorizontal = this->getDepth();
			float ballRadius = ball.getRadius();

			//each represents the boundary
			float wallUp = wallPosX - wallVertical * 0.5;
			float wallLow = wallPosX + wallVertical * 0.5;
			float wallLeft = wallPosZ - wallHorizontal * 0.5;
			float wallRight = wallPosZ + wallHorizontal * 0.5;
			
			if (!(wallUp <= ballPosX && ballPosX <= wallLow) && (wallLeft <= ballPosZ && ballPosZ <= wallRight)) {
				
				ball.setPower(-vx, vz);

				if (wallLeft - ballRadius <= ballPosX && ballPosX <= wallPosX) {
					ballPosX = wallUp - ballRadius;
				}
				else {
					ballPosX = wallLow + ballRadius;
				}
				
			}

			if ((wallUp <= ballPosX && ballPosX <= wallLow) && !(wallLeft <= ballPosZ && ballPosZ <= wallRight)) {
				
				ball.setPower(vx, -vz);

				if (wallUp - ballRadius <= ballPosZ && ballPosZ <= wallPosZ) {
					ballPosZ = wallLeft - ballRadius;
				}
				else {
					ballPosZ = wallRight + ballRadius;
				}
				
			}

			ball.setCenter(ballPosX, ballPosY, ballPosZ);
		}
	}

	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	D3DXVECTOR3 getPosition(void) const {
		D3DXVECTOR3 wallPos(m_x, 0 ,m_z);
		return wallPos;
	}

	float getHeight(void) const { return M_HEIGHT; }
	float getWidth(void) const {return m_width; }
	float getDepth(void) const { return m_depth; }

	
	
private :
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	
	D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pBoundMesh;
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
    CLight(void)
    {
        static DWORD i = 0;
        m_index = i++;
        D3DXMatrixIdentity(&m_mLocal);
        ::ZeroMemory(&m_lit, sizeof(m_lit));
        m_pMesh = NULL;
        m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_bound._radius = 0.0f;
    }
    ~CLight(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
    {
        if (NULL == pDevice)
            return false;
        if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
            return false;
		
        m_bound._center = lit.Position;
        m_bound._radius = radius;
		
        m_lit.Type          = lit.Type;
        m_lit.Diffuse       = lit.Diffuse;
        m_lit.Specular      = lit.Specular;
        m_lit.Ambient       = lit.Ambient;
        m_lit.Position      = lit.Position;
        m_lit.Direction     = lit.Direction;
        m_lit.Range         = lit.Range;
        m_lit.Falloff       = lit.Falloff;
        m_lit.Attenuation0  = lit.Attenuation0;
        m_lit.Attenuation1  = lit.Attenuation1;
        m_lit.Attenuation2  = lit.Attenuation2;
        m_lit.Theta         = lit.Theta;
        m_lit.Phi           = lit.Phi;
        return true;
    }
    void destroy(void)
    {
        if (m_pMesh != NULL) {
            m_pMesh->Release();
            m_pMesh = NULL;
        }
    }
    bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return false;
		
        D3DXVECTOR3 pos(m_bound._center);
        D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
        D3DXVec3TransformCoord(&pos, &pos, &mWorld);
        m_lit.Position = pos;
		
        pDevice->SetLight(m_index, &m_lit);
        pDevice->LightEnable(m_index, TRUE);
        return true;
    }

    void draw(IDirect3DDevice9* pDevice)
    {
        if (NULL == pDevice)
            return;
        D3DXMATRIX m;
        D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
        pDevice->SetTransform(D3DTS_WORLD, &m);
        pDevice->SetMaterial(&d3d::WHITE_MTRL);
        m_pMesh->DrawSubset(0);
    }

    D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
    DWORD               m_index;
    D3DXMATRIX          m_mLocal;
    D3DLIGHT9           m_lit;
    ID3DXMesh*          m_pMesh;
    d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[3];
CSphere	g_targetBall[BALL_NUM]; //target Yellow balls, destroyed when hit by the red ball, g_destroyerBall
CSphere	g_hitterBall; //white ball that hits red ball 
CSphere g_destroyerBall; // red ball that hits target yellow balls
CLight	g_light;

double g_camera_pos[3] = {0.0, -5.0, 8.0};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
	int i;
	
    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);
		
	// create plane and set the position
	if (false == g_legoPlane.create(Device, -1, -1, 10, 0.03f, 8, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);
	
	// create walls and set the position. note that there are three walls
	if (false == g_legowall[0].create(Device, -1, -1, 10, 0.6f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.24f, 4.06f);
	if (false == g_legowall[1].create(Device, -1, -1, 10, 0.6f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(0.0f, 0.24f, -4.06f);
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.6f, 8.24f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(-5.06f, 0.24f, 0.0f);
	
	

	// create target BALL_NUM balls and set the position
	for (i=0;i< BALL_NUM;i++) {
		if (false == g_targetBall[i].create(Device, d3d::YELLOW)) return false;
		g_targetBall[i].setCenter(spherePos[i][0], (float)M_RADIUS , spherePos[i][1]);
		g_targetBall[i].setPower(0,0);
	}
	
	// create white ball for hitting the red ball
    if (false == g_hitterBall.create(Device, d3d::WHITE)) return false;
	g_hitterBall.setCenter(g_legoPlane.getWidth()*0.5, 0.5, M_RADIUS);
	g_hitterBall.designateHitter();
	
	// create red ball for destroying target balls
	if (false == g_destroyerBall.create(Device, d3d::RED)) return false;
	g_destroyerBall.setCenter(g_legoPlane.getWidth() * 0.5, M_RADIUS, M_RADIUS);
	
	// light setting 
    D3DLIGHT9 lit;
    ::ZeroMemory(&lit, sizeof(lit));
    lit.Type         = D3DLIGHT_POINT;
    lit.Diffuse      = d3d::WHITE; 
	lit.Specular     = d3d::WHITE * 0.9f;
    lit.Ambient      = d3d::WHITE * 0.9f;
    lit.Position     = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
    lit.Range        = 100.0f;
    lit.Attenuation0 = 0.0f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;
    if (false == g_light.create(Device, lit))
        return false;
	
	// Position and aim the camera.
	D3DXVECTOR3 pos(10.0f, 10.0f, 0.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);
	
	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
        (float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);
	
    // Set render states.
    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	
	g_light.setLight(Device, g_mWorld);
	return true;
}

void Cleanup(void)
{
    g_legoPlane.destroy();
	for(int i = 0 ; i < 3; i++) {
		g_legowall[i].destroy();
	}
    destroyAllLegoBlock();
    g_light.destroy();
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	int i=0;

	if( Device )
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();
		
		if (notYetStart) {
			D3DXVECTOR3 hitterBallCoord = g_hitterBall.getCenter();
			g_destroyerBall.setCenter(hitterBallCoord.x - 2 * M_RADIUS, hitterBallCoord.y , hitterBallCoord.z);
		}
		// update the position of each ball. during update, check whether each ball hit by walls.
		
		else{
			/*update balls*/
			for (i = 0; i < BALL_NUM; i++) {
				g_targetBall[i].ballUpdate(timeDelta);
			}
			g_destroyerBall.ballUpdate(timeDelta);
			g_hitterBall.ballUpdate(timeDelta);
		

			for (i = 0; i < BALL_NUM; i++) {//destroyer ball hitting target balls check
				g_targetBall[i].hitBy(g_destroyerBall);
			}
			
			for (i = 0; i < 3; i++) {//destroyer ball hitting walls check
				g_legowall[i].hitBy(g_destroyerBall);
			}

			g_hitterBall.hitBy(g_destroyerBall); //destroyer ball hitting hitter ball check
		}
		
		//Ball out of the plane --> reset
		if (g_destroyerBall.getCenter().x >= g_legoPlane.getPosition().x + g_legoPlane.getWidth() * 0.5) {
			g_destroyerBall.setPower(0, 0);

			D3DXVECTOR3 hitterBallCoord = g_hitterBall.getCenter();
			g_destroyerBall.setCenter(hitterBallCoord.x - 2 * M_RADIUS, hitterBallCoord.y, hitterBallCoord.z);
			
			for (i = 0; i < BALL_NUM; i++) {
				g_targetBall[i].setCenter(spherePos[i][0], M_RADIUS, spherePos[i][1]);
				g_targetBall[i].setPower(0, 0);
			}

			notYetStart = true;	
		}

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);//plane
		for (i=0;i<3;i++) 	{//walls
			g_legowall[i].draw(Device, g_mWorld);
		}

		for (i = 0;i< BALL_NUM;i++) {//Yellow balls
			g_targetBall[i].draw(Device, g_mWorld);
		}
		g_hitterBall.draw(Device, g_mWorld);//White ball
		g_destroyerBall.draw(Device, g_mWorld);//Red ball
        g_light.draw(Device);
		
		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture( 0, NULL );
	}
	return true;
}


LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
	static int old_x = 0;
	static int old_y = 0;
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

	switch (msg) {
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_ESCAPE:
			::DestroyWindow(hwnd);
			break;
		case VK_RETURN:
			if (NULL != Device) {
				wire = !wire;
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
			}
			break;
		case VK_SPACE:

			if (notYetStart) {
				notYetStart = false;
			}
			g_destroyerBall.setPower(-2.0, 0.0);
		}
		break;
	}

	case WM_MOUSEMOVE:
	{
		int new_x = LOWORD(lParam);
		int new_y = HIWORD(lParam);
		float dx;
		float dy;
		float new_z;

		/*left and right wall boundary with margin of radius */
		float wallRight = g_legowall[0].getPosition().z - g_legowall[0].getDepth() * 0.5 - g_hitterBall.getRadius();
		float wallLeft = g_legowall[1].getPosition().z + g_legowall[1].getDepth() * 0.5 + g_hitterBall.getRadius();
		
		

		if (LOWORD(wParam) & MK_LBUTTON) {
			D3DXVECTOR3 ballPos = g_hitterBall.getCenter();
			dx = (old_x - new_x);
			dy = (old_y - new_y);
			new_z = ballPos.z + dx * (-0.007f);

			if (new_z < wallLeft) {
				new_z = wallLeft;
			}			
			else if (new_z > wallRight) {
				new_z = wallRight;
			}

			g_hitterBall.setCenter(ballPos.x, ballPos.y, new_z);
			old_x = new_x;
			old_y = new_y;

			move = WORLD_MOVE;
			
		}
		break;
	}
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));
	
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
	
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}
	
	d3d::EnterMsgLoop( Display );
	
	Cleanup();
	
	Device->Release();
	
	return 0;
}