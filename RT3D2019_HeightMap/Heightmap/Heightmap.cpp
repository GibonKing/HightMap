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
	XMVECTOR getAvgNormal(const std::vector<XMFLOAT3>&, const std::vector<XMFLOAT3>&, int, int, int, int);
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
	int normalCount = width * height * 3;
	std::vector<XMFLOAT3> topNormals(normalCount);
	std::vector<XMFLOAT3> botNormals(normalCount);

	int topLeft;
	int top;
	int topRight;
	int left;
	int centre;
	int right;
	int botLeft;
	int bot;
	int botRight;
	int miTopLeft;
	int miTop;
	int miTopRight;
	int miLeft;
	int miCentre;
	int miRight;
	int miBotLeft;
	int miBot;
	int miBotRight;


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

			//Vectors between points
			XMVECTOR V0V1 = XMLoadFloat3(&V1) - XMLoadFloat3(&V0);
			XMVECTOR V0V2 = XMLoadFloat3(&V2) - XMLoadFloat3(&V0);
			XMVECTOR V3V1 = XMLoadFloat3(&V1) - XMLoadFloat3(&V3);
			XMVECTOR V3V2 = XMLoadFloat3(&V2) - XMLoadFloat3(&V3);

			//Normal Vectors
			XMVECTOR TNV = XMVector3Cross(V3V1, V3V2);
			XMVECTOR BNV = XMVector3Cross(-V0V1, V0V2);

			//Normal Floats
			XMFLOAT3 TNF;
			XMStoreFloat3(&TNF, TNV);
			XMFLOAT3 BNF;
			XMStoreFloat3(&BNF, BNV);

			//Add normals to array
			topNormals[mapIndex] = TNF;
			botNormals[mapIndex] = BNF;
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

			//Map Indexes
			mapIndex = (gridSqreZ * m_HeightMapWidth) + useGridSqrX;
			miTopLeft = mapIndex - m_HeightMapWidth - 1;
			miTop = mapIndex - m_HeightMapWidth;
			miTopRight = mapIndex - m_HeightMapWidth + 1;
			miLeft = mapIndex - 1;
			miCentre = mapIndex;
			miRight = mapIndex + 1;
			miBotLeft = mapIndex + m_HeightMapWidth - 1;
			miBot = mapIndex + m_HeightMapWidth;
			miBotRight = mapIndex + m_HeightMapWidth + 1;

			//Vertices
			XMFLOAT3 V0 = m_pHeightMap[mapIndex + m_HeightMapWidth];		//BottomLeft
			XMFLOAT3 V1 = m_pHeightMap[mapIndex];							//TopLeft
			XMFLOAT3 V2 = m_pHeightMap[mapIndex + m_HeightMapWidth + 1];	//BottomRight
			XMFLOAT3 V3 = m_pHeightMap[mapIndex + 1];						//TopRight

			//Check Array Values
			topLeft	 = miTopLeft	>= 0 ? miTopLeft	< normalCount ? miTopLeft	: -1 : -1;
			top		 = miTop		>= 0 ? miTop		< normalCount ? miTop		: -1 : -1;
			topRight = miTopRight	>= 0 ? miTopRight	< normalCount ? miTopRight	: -1 : -1;
			left	 = miLeft		>= 0 ? miLeft		< normalCount ? miLeft		: -1 : -1;
			centre	 = miCentre		>= 0 ? miCentre		< normalCount ? miCentre	: -1 : -1;
			right	 = miRight		>= 0 ? miRight		< normalCount ? miRight		: -1 : -1;
			botLeft	 = miBotLeft	>= 0 ? miBotLeft	< normalCount ? miBotLeft	: -1 : -1;
			bot		 = miBot		>= 0 ? miBot		< normalCount ? miBot		: -1 : -1;
			botRight = miBotRight	>= 0 ? miBotRight	< normalCount ? miBotRight	: -1 : -1;
			
			XMVECTOR avgV2V = getAvgNormal(botNormals, topNormals, centre, right, bot, botRight); //Average Normal for Bottom Right
			XMFLOAT3 avgV2;
			XMStoreFloat3(&avgV2, avgV2V);

			//Put plots into array
			if (Even) {

				XMVECTOR avgV3V = getAvgNormal(botNormals, topNormals, top, topRight, centre, right); //Average Normal for Top Right
				XMFLOAT3 avgV3;
				XMStoreFloat3(&avgV3, avgV3V);

				if (useGridSqrX == 0) {

					XMVECTOR avgV0V = getAvgNormal(botNormals, topNormals, left, centre, botLeft, bot); //Average Normal for Bottom Left
					XMFLOAT3 avgV0;
					XMStoreFloat3(&avgV0, avgV0V);

					XMVECTOR avgV1V = getAvgNormal(botNormals, topNormals, topLeft, top, left, centre); //Average Normal for Top Left
					XMFLOAT3 avgV1;
					XMStoreFloat3(&avgV1, avgV1V);

					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V0, MAP_COLOUR, avgV0);
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V1, MAP_COLOUR, avgV1);
				}

				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V2, MAP_COLOUR, avgV2);
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V3, MAP_COLOUR, avgV3);

				if (useGridSqrX == width - 1) {
					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V2, MAP_COLOUR, avgV2);
				}
			}
			else {

				XMVECTOR avgV1V = getAvgNormal(botNormals, topNormals, topLeft, top, left, centre); //Average Normal for Top Left
				XMFLOAT3 avgV1;
				XMStoreFloat3(&avgV1, avgV1V);

				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V2, MAP_COLOUR, avgV2);
				m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V1, MAP_COLOUR, avgV1);

				if (useGridSqrX == 0) {

					XMVECTOR avgV0V = getAvgNormal(botNormals, topNormals, left, centre, botLeft, bot); //Average Normal for Bottom Left
					XMFLOAT3 avgV0;
					XMStoreFloat3(&avgV0, avgV0V);

					m_pMapVtxs[vertex++] = Vertex_Pos3fColour4ubNormal3f(V0, MAP_COLOUR, avgV0);
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

XMVECTOR HeightMapApplication::getAvgNormal(const std::vector<XMFLOAT3>& topNormals, const std::vector<XMFLOAT3>& botNormals, int sqr1, int sqr2, int sqr3, int sqr4) {
	XMFLOAT3 zero(0.0f, 0.0f, 0.0f);

	XMVECTOR avgNormal = (
		(sqr1 == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[sqr1]) + XMLoadFloat3(&topNormals[sqr1]))) +
		(sqr2 == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[sqr2]))) +
		(sqr3 == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&topNormals[sqr3]))) +
		(sqr4 == -1 ? XMVECTOR(XMLoadFloat3(&zero)) : XMVECTOR(XMLoadFloat3(&botNormals[sqr4]) + XMLoadFloat3(&topNormals[sqr4])))
		) / 6;	

	return avgNormal;
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
