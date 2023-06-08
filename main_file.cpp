/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "myCube.h"
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

float aspectRatio = 1;
GLuint tex;

float speed_x = 0;
float speed_y = 0;
float speed_z = 0;
float angle_y = 0;
float speed = 25;
// Camera
float cameraDistance = 20;
float cameraAngle = 0;
float cameraHeight = 5;

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}



void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) { speed_z = speed; angle_y = 0.5f; }// tutaj obrocw lewo
		if (key == GLFW_KEY_RIGHT){ speed_z = -speed; angle_y = -0.5f; }// tutaj obroc w prawo
		if (key == GLFW_KEY_UP) speed_x = -speed;
		if (key == GLFW_KEY_DOWN) speed_x = speed;
		if (key == GLFW_KEY_X) speed_y = -speed;
		if (key == GLFW_KEY_Z) speed_y = speed;
		if (key == GLFW_KEY_KP_SUBTRACT) cameraDistance -= 5;
		if (key == GLFW_KEY_KP_ADD) cameraDistance += 5;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_z = 0; angle_y = 0;
		if (key == GLFW_KEY_RIGHT) speed_z = 0; angle_y = 0;
		if (key == GLFW_KEY_UP) speed_x = 0;
		if (key == GLFW_KEY_DOWN) speed_x = 0;
		if (key == GLFW_KEY_X) speed_y = 0;
		if (key == GLFW_KEY_Z) speed_y = 0;
	}
}

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

class Model {
public:
	std::vector<glm::vec4> verts;
	std::vector<glm::vec4> colors;
	std::vector<glm::vec4> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<unsigned int> indices;
};

std::vector<Model> loadModel(std::string plik) 
{
	using namespace std;
	vector <Model> models;
	
	vector<glm::vec4> modelVerts;
	vector<glm::vec4> modelColors;
	vector<glm::vec4> modelNormals;
	vector<glm::vec2> modelTexCoords;
	vector<unsigned int> modelIndices;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
	cout << importer.GetErrorString() << endl << "Meshes: " << scene->mNumMeshes << endl << "Materials: " << scene->mNumMaterials << endl << "Textures: " << scene->mNumTextures << endl << "Lights: " << scene->mNumLights << endl << "Cameras: " << scene->mNumCameras << endl;
	
	for (int k = 0; k < scene->mNumMeshes; k++) 
	{
		Model model;
		aiMesh* mesh = scene->mMeshes[k];
		//cout << "Vertices: " << mesh->mNumVertices << endl << "Faces: " << mesh->mNumFaces << endl << "Texture channels: " << mesh->GetNumUVChannels() << endl << "Color channels: " << mesh->GetNumColorChannels() << endl;
		for (int i = 0; i < mesh->mNumVertices; i++) 
		{
			aiVector3D vertex = mesh->mVertices[i]; // aiVector3D podobny do glm::vec3
			modelVerts.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));
			aiVector3D normal = mesh->mNormals[i]; // Wektory znormalizowane
			modelNormals.push_back(glm::vec4(normal.x, normal.y, normal.z, 0)); // 0, bo wektor
			unsigned int liczba_zest = mesh->GetNumUVChannels(); // liczba zdefiniowanych zestawów wsp. teksturowania (zestawów jest max 8)
			unsigned int wymiar_wsp_tex = mesh->mNumUVComponents[0]; // Liczba składowych wsp. teksturowania dla 0 zestawu.
			if (liczba_zest > 0 && wymiar_wsp_tex > 0) 
			{
				aiVector3D texCoord = mesh->mTextureCoords[0][i]; // 0 to numer zestawu współrzędnych teksturowania
				modelTexCoords.push_back(glm::vec2(texCoord.x, texCoord.y)); // x,y,z wykorzystywane jako u,v,w. 0 jeżeli tekstura ma mniej wymiarów
			}
		}

		//dla każdego wielokąta składowego
		for (int i = 0; i < mesh->mNumFaces; i++) 
		{
			aiFace& face = mesh->mFaces[i]; //face to jeden z wielokątów siatki
			//dla każdego indeksu->wierzchołka tworzącego wielokąt
			//dla aiProcess_Triangulate to zawsze będzie 3
			for (int j = 0; j < face.mNumIndices; j++) 
			{
				modelIndices.push_back(face.mIndices[j]);
			}
		}
		model.verts = modelVerts;
		model.colors = modelColors;
		model.normals = modelNormals;
		model.texCoords = modelTexCoords;
		model.indices = modelIndices;
		models.push_back(model);
	}
	return models;
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    initShaders();
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0, 0, 0, 1); //Ustaw kolor czyszczenia bufora kolorów
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	tex = readTexture("models/textures/trasa.png");
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    freeShaders();
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
	glDeleteTextures(1, &tex);
}


void kostka(glm::mat4 P, glm::mat4 V, glm::mat4 M) {
	spColored->use(); //Aktywuj program cieniujący

	glUniformMatrix4fv(spColored->u("P"), 1, false, glm::value_ptr(P)); //Załaduj do programu cieniującego macierz rzutowania
	glUniformMatrix4fv(spColored->u("V"), 1, false, glm::value_ptr(V)); //Załaduj do programu cieniującego macierz widoku
	glUniformMatrix4fv(spColored->u("M"), 1, false, glm::value_ptr(M)); //Załaduj do programu cieniującego macierz modelu

	glEnableVertexAttribArray(spColored->a("vertex"));
	glVertexAttribPointer(spColored->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices); //Współrzędne wierzchołków bierz z tablicy birdVertices

	glEnableVertexAttribArray(spColored->a("color"));
	glVertexAttribPointer(spColored->a("color"), 4, GL_FLOAT, false, 0, myCubeColors); //Współrzędne wierzchołków bierz z tablicy birdColors

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

	glDisableVertexAttribArray(spColored->a("vertex"));
	glDisableVertexAttribArray(spColored->a("color"));
}

void drawModel(glm::mat4 P, glm::mat4 V, glm::mat4 M, std::vector<Model> models) {
	spTextured->use();
	std::cout << models.size() << std::endl;
	for (int i = 0; i < models.size(); i++) 
	{
		glUniformMatrix4fv(spTextured->u("P"), 1, false, glm::value_ptr(P)); //Załaduj do programu cieniującego macierz rzutowania
		glUniformMatrix4fv(spTextured->u("V"), 1, false, glm::value_ptr(V)); //Załaduj do programu cieniującego macierz widoku
		glUniformMatrix4fv(spTextured->u("M"), 1, false, glm::value_ptr(M)); //Załaduj do programu cieniującego macierz modelu
		// Przypisz dane
		glEnableVertexAttribArray(spTextured->a("vertex"));  // Włącz atrybut vertex
		glVertexAttribPointer(spTextured->a("vertex"), 4, GL_FLOAT, false, 0, models[i].verts.data());
		glEnableVertexAttribArray(spTextured->a("normal"));  // Włącz atrybut normal
		glVertexAttribPointer(spTextured->a("normal"), 4, GL_FLOAT, false, 0, models[i].normals.data());
		glEnableVertexAttribArray(spTextured->a("texCoord"));  // Włącz atrybut texCoord
		glVertexAttribPointer(spTextured->a("texCoord"), 2, GL_FLOAT, false, 0, models[i].texCoords.data());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glGenerateMipmap(GL_TEXTURE_2D);
		glUniform1i(spTextured->u("tex"), 0);
		glDrawElements(GL_TRIANGLES, models[i].indices.size(), GL_UNSIGNED_INT, models[i].indices.data());
		// Clear vertex attribute state
		glDisableVertexAttribArray(spTextured->a("vertex"));  // Disable vertex attribute
		glDisableVertexAttribArray(spTextured->a("normal"));  // Disable normal attribute
		glDisableVertexAttribArray(spTextured->a("texCoord"));  // Disable normal attribute
	}
	
}

void printMatrix(const glm::mat4& matrix) {
	std::cout << "--------------" << std::endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << matrix[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float carX, float carY, float carZ, float angle_y, std::vector<Model> floor, std::vector<Model> car) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości
	glm::vec3 lookPoint = glm::vec3(carX - cameraDistance, carY, carZ);
	glm::mat4 P = glm::perspective(30.0f * PI / 180.0f, aspectRatio, 0.01f, 1200.0f); //Wylicz macierz rzutowania
	glm::mat4 V = glm::lookAt(
		glm::vec3(carX, cameraHeight, carZ),  // Pozycja kamery
		lookPoint,  // Punkt patrzenia
		glm::vec3(0, 1, 0));
	// Rysowanie kostki
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::translate(M, lookPoint);
	//V = glm::rotate(M, angle_y, glm::vec3(0, 1, 0));
	M = glm::rotate(M, -PI/2+angle_y, glm::vec3(0, 1, 0));
	
	//printMatrix(M);
	drawModel(P, V, M, car);
	//kostka(P, V, M);
	// Rysowanie terenu
	M = glm::mat4(1.0f); //Macierz jednostkowa
	M = glm::translate(M, glm::vec3(0, -3, 0)); // OBNIZENIE POZIOMU TERENU
	drawModel(P, V, M, floor); //Rysowanie terenu
	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno
	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów
	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}
	window = glfwCreateWindow(800, 800, "Drive Simulator", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.
	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora
	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}
	initOpenGLProgram(window); //Operacje inicjujące
	
	float carX = 0; 
	float carY = 0; 
	float carZ = 0;
	float tmp = 0;
	float tmp_pi = 0;
	std::vector<Model> floor = loadModel("models/test3.obj");
	std::vector<Model> car = loadModel("models/car.obj");
	glfwSetTime(0); 
	//Główna pętla
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		
		carX += speed_x * glfwGetTime();
		carY += speed_y * glfwGetTime();
		carZ += speed_z * glfwGetTime();
		tmp += angle_y * glfwGetTime();
		tmp_pi = tmp / 3.14;
		//std::cout << "KAMERA | x: " << carX << " | y: " << carY << " | z: " << carZ << " || MODEL: kat rotacji = ";
		//std::cout << tmp/3.14*90 << " stopni." << std::endl;
		glfwSetTime(0); //Zeruj timer
		drawScene(window, carX, carY, carZ, tmp, floor, car); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
