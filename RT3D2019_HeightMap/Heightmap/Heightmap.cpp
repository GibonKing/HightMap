#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>

#include "CommonApp.h"

#include <stdio.h>

#include <DirectXMath.h>
using namespace DirectX;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class HeightMapApplication : public CommonApp
{
  public:
  protected:
	bool HandleStart();
	void HandleStop();
	void HandleUpdate();
	void HandleRender();
	bool LoadHeightMap(char* filename, float gridSize);

  private:
	ID3D11Buffer* m_pHeightMapBuffer;
	float m_rotationAngle;
	int m_HeightMapWidth;
	int m_HeightMapLength;
	int m_HeightMapVtxCount;
	XMFLOAT3* m_pHeightMap;
	Vertex_Pos3fColour4ubNormal3f* m_pMapVtxs;
	float m_cameraZ;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool HeightMapApplication::HandleStart()
{
	this->SetWindowTitle("HeightMap");

	LoadHeightMap("HeightMap.bmp", 1.0f);

	m_cameraZ = 50.0f;

	m_pHeightMapBuffer = NULL;

	m_rotationAngle = 0.f;

	if(!this->CommonApp::HandleStart())
		return false;

	static const VertexColour MAP_COLOUR(200, 255, 255, 255);

	/////////////////////////////////////////////////////////////////
	// Clearly this code will need changing to render the heightmap
	/////////////////////////////////////////////////////////////////

	m_HeightMapVtxCount =  m_HeightMapLength * m_HeightMapWidth * 2;
	m_pMapVtxs = new Vertex_Pos3fColour4ubNormal3f[m_HeightMapVtxCount];

	int vertex(0);
	int normal(0);
	int width = m_HeightMapWidth - 1;
	int height = m_HeightMapLength - 1;
	bool Even = true;
	int useGridSqrX;
	int mapIndex;
	int test = 0;
	std::vector<XMFLOAT3> normals(width * height * 3);


	for (int gridSqreZ(0); gridSqreZ < height; gridSqreZ++) {
		test++;
		for (int gridSqrX(0); gridSqrX < width; gridSqrX++) {

			if (Even)
				useGridSqrX = gridSqrX;
			else
				useGridSqrX = (width - 1) - gridSqrX;

			mapIndex = (gridSqreZ * m_HeightMapWidth) + useGridSqrX;

			//Vertices
			XMFLOAT3 V1 = m_pHeightMap[mapIndex];							//TopLeft
			XMFLOAT3 V2 = m_pHeightMap[mapIndex + m_HeightMapWidth + 1];	//BottomRight
			XMFLOAT3 V3 = m_pHeightMap[mapIndex + 1];						//TopRight

			//Vectors between points
			XMVECTOR V3V2 = XMLoadFloat3(&V2) - XMLoadFloat3(&V3);
			XMVECTOR V3V1 = XMLoadFloat3(&V1) - XMLoadFloat3(&V3);

			//Normal Vectors
			XMVECTOR N1V = XMVector3Cross(-V3V2, V3V1);

			//Normal Floats
			XMFLOAT3 N1F;
			XMStoreFloat3(&N1F, N1V);

			//Add to normals array
			normals[mapIndex] = N1F;
		}
		Even = !Even;
	}

	normal = 0;

	for (int gridSqreZ(0); gridSqreZ < height; gridSqreZ++) {
		for (int gridSqrX(0); gridSqrX < width; gridSqrX++) {

			if (Even)
				useGridSqrX = gridSqrX;
			else
				useGridSqrX = (width - 1) - gridSqrX;

			mapIndex = (gridSqreZ * m_HeightMapWidth) + useGridSqrX;

			//Vertices
			XMFLOAT3 V0 = m_pHeightMap[mapIndex + m_HeightMapWidth];		//BottomLeft
			XMFLOAT3 V1 = m_pHeightMap[mapIndex];							//TopLeft
			XMFLOAT3 V2 = m_pHeightMap[mapIndex + m_HeightMapWidth + 1];	//BottomRight
			XMFLOAT3 V3 = m_pHeightMap[mapIndex + 1];						//TopRight

			//Put plots into array
			if (Even) {
				if (useGridSqrX == 0) {
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V0, MAP_COLOUR, normals[mapIndex + m_HeightMapWidth]);
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V1, MAP_COLOUR, normals[mapIndex]);
				}
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V2, MAP_COLOUR, normals[mapIndex]);
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V3, MAP_COLOUR, normals[mapIndex]);
				if (useGridSqrX == width - 1) {
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V2, MAP_COLOUR, normals[mapIndex]);
				}
			}
			else {
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V2, MAP_COLOUR, normals[mapIndex]);
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V1, MAP_COLOUR, normals[mapIndex]);
				if (useGridSqrX == 0) {
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V0, MAP_COLOUR, normals[mapIndex + m_HeightMapWidth]);
				}
			}
		}
		Even = !Even;
	}

	/////////////////////////////////////////////////////////////////
	// Down to here
	/////////////////////////////////////////////////////////////////

	m_pHeightMapBuffer = CreateImmutableVertexBuffer(m_pD3DDevice, sizeof Vertex_Pos3fColour4ubNormal3f * m_HeightMapVtxCount, m_pMapVtxs);

 	delete m_pMapVtxs;

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMapApplication::HandleStop()
{
	Release(m_pHeightMapBuffer);

	this->CommonApp::HandleStop();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMapApplication::HandleUpdate()
{
	m_rotationAngle += .01f;

	if(this->IsKeyPressed('Q'))
	{
		if(m_cameraZ > 20.0f)
			m_cameraZ -= 2.0f;
	}

	if(this->IsKeyPressed('A'))
	{
		m_cameraZ += 2.0f;
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMapApplication::HandleRender()
{
	XMFLOAT3 vCamera(sin(m_rotationAngle) * m_cameraZ, m_cameraZ / 2, cos(m_rotationAngle) * m_cameraZ);
	XMFLOAT3 vLookat(0.0f, 0.0f, 0.0f);
	XMFLOAT3 vUpVector(0.0f, 1.0f, 0.0f);

	XMMATRIX matView;
	matView = XMMatrixLookAtLH(XMLoadFloat3(&vCamera), XMLoadFloat3(&vLookat), XMLoadFloat3(&vUpVector));

	XMMATRIX matProj;
	matProj = XMMatrixPerspectiveFovLH(float(XM_PI / 4), 2, 1.5f, 5000.0f);

	this->SetViewMatrix(matView);
	this->SetProjectionMatrix(matProj);

	this->EnablePointLight(0, XMFLOAT3(100.0f, 100.f, -100.f), XMFLOAT3(1.f, 1.f, 1.f));

	this->Clear(XMFLOAT4(.2f, .2f, .6f, 1.f));

	this->DrawUntexturedLit(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pHeightMapBuffer, NULL, m_HeightMapVtxCount);
}

//////////////////////////////////////////////////////////////////////
// LoadHeightMap
// Original code sourced from rastertek.com
//////////////////////////////////////////////////////////////////////
bool HeightMapApplication::LoadHeightMap(char* filename, float gridSize)
{
	FILE* filePtr;
	int error;
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;

	// Open the height map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if(error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	m_HeightMapWidth = bitmapInfoHeader.biWidth;
	m_HeightMapLength = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = m_HeightMapWidth * m_HeightMapLength * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if(!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if(count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if(error != 0)
	{
		return false;
	}

	// Create the structure to hold the height map data.
	m_pHeightMap = new XMFLOAT3[m_HeightMapWidth * m_HeightMapLength];
	if(!m_pHeightMap)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k = 0;

	// Read the image data into the height map.
	for(j = 0; j < m_HeightMapLength; j++)
	{
		for(i = 0; i < m_HeightMapWidth; i++)
		{
			height = bitmapImage[k];

			index = (m_HeightMapLength * j) + i;

			m_pHeightMap[index].x = (float)(i - (m_HeightMapWidth / 2)) * gridSize;
			m_pHeightMap[index].y = (float)height / 16 * gridSize;
			m_pHeightMap[index].z = (float)(j - (m_HeightMapLength / 2)) * gridSize;

			k += 3;
		}
	}

	// Release the bitmap image data.
	delete[] bitmapImage;
	bitmapImage = 0;

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	HeightMapApplication application;

	Run(&application);

	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
