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

//Normalnie użyć klasy 3D z atrybutami. Tutaj uproszczone
std::vector<glm::vec4> vertss;
std::vector<glm::vec4> colorss;
std::vector<glm::vec4> normalss;
std::vector<glm::vec2> texCoordss;
std::vector<unsigned int> indicess;

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_x = -50;
		if (key == GLFW_KEY_RIGHT) speed_x = 50;
		if (key == GLFW_KEY_UP) speed_z = 50;
		if (key == GLFW_KEY_DOWN) speed_z = -50;
		if (key == GLFW_KEY_X) speed_y = -50;
		if (key == GLFW_KEY_Z) speed_y = 50;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_x = 0;
		if (key == GLFW_KEY_RIGHT) speed_x = 0;
		if (key == GLFW_KEY_UP) speed_z = 0;
		if (key == GLFW_KEY_DOWN) speed_z = 0;
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

void loadModel(std::string plik) {
	using namespace std;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
	cout << importer.GetErrorString() << endl;
	cout << "Meshes: " << scene->mNumMeshes << endl;
	cout << "Materials: " << scene->mNumMaterials << endl;
	cout << "Textures: " << scene->mNumTextures << endl;
	cout << "Lights: " << scene->mNumLights << endl;
	cout << "Cameras: " << scene->mNumCameras << endl;

	for (int k = 0; k < scene->mNumMeshes; k++) {

		aiMesh* mesh = scene->mMeshes[k];
		cout << "Vertices: " << mesh->mNumVertices << endl;
		cout << "Faces: " << mesh->mNumFaces << endl;
		cout << "Texture channels: " << mesh->GetNumUVChannels() << endl;
		cout << "Color channels: " << mesh->GetNumColorChannels() << endl;
		for (int i = 0; i < mesh->mNumVertices; i++) {
			aiVector3D vertex = mesh->mVertices[i]; // aiVector3D podobny do glm::vec3
			vertss.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));

			aiVector3D normal = mesh->mNormals[i]; // Wektory znormalizowane
			normalss.push_back(glm::vec4(normal.x, normal.y, normal.z, 0)); // 0, bo wektor

			unsigned int liczba_zest = mesh->GetNumUVChannels(); // liczba zdefiniowanych zestawów wsp. teksturowania (zestawów jest max 8)
			unsigned int wymiar_wsp_tex = mesh->mNumUVComponents[0]; // Liczba składowych wsp. teksturowania dla 0 zestawu.

			if (liczba_zest > 0 && wymiar_wsp_tex > 0) {
				aiVector3D texCoord = mesh->mTextureCoords[0][i]; // 0 to numer zestawu współrzędnych teksturowania
				texCoordss.push_back(glm::vec2(texCoord.x, texCoord.y)); // x,y,z wykorzystywane jako u,v,w. 0 jeżeli tekstura ma mniej wymiarów
			}

			// Dodaj kolory
			if (mesh->HasVertexColors(0)) {
				aiColor4D color = mesh->mColors[0][i];
				colorss.push_back(glm::vec4(color.r, color.g, color.b, color.a));
			}
			else {
				colorss.push_back(glm::vec4(1, 1, 1, 1));
			}
		}

		//dla każdego wielokąta składowego
		for (int i = 0; i < mesh->mNumFaces; i++) {
			aiFace& face = mesh->mFaces[i]; //face to jeden z wielokątów siatki
			//dla każdego indeksu->wierzchołka tworzącego wielokąt
			//dla aiProcess_Triangulate to zawsze będzie 3
			for (int j = 0; j < face.mNumIndices; j++) {
				indicess.push_back(face.mIndices[j]);
			}
		}
	}
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
	tex = readTexture("models/textures/gradient.png");
	loadModel("models/source/mercedesf1.obj");
	std::cout << indicess.size();
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

void testModel(glm::mat4 P, glm::mat4 V, glm::mat4 M) {
	spTextured->use();
	glUniformMatrix4fv(spTextured->u("P"), 1, false, glm::value_ptr(P)); //Załaduj do programu cieniującego macierz rzutowania
	glUniformMatrix4fv(spTextured->u("V"), 1, false, glm::value_ptr(V)); //Załaduj do programu cieniującego macierz widoku
	glUniformMatrix4fv(spTextured->u("M"), 1, false, glm::value_ptr(M)); //Załaduj do programu cieniującego macierz modelu
	// Przypisz dane
	glEnableVertexAttribArray(spTextured->a("vertex"));  // Włącz atrybut vertex
	glVertexAttribPointer(spTextured->a("vertex"), 4, GL_FLOAT, false, 0, vertss.data()); 
	glEnableVertexAttribArray(spTextured->a("normal"));  // Włącz atrybut normal
	glVertexAttribPointer(spTextured->a("normal"), 4, GL_FLOAT, false, 0, normalss.data());
	glEnableVertexAttribArray(spTextured->a("texCoord"));  // Włącz atrybut texCoord
	glVertexAttribPointer(spTextured->a("texCoord"), 2, GL_FLOAT, false, 0, texCoordss.data());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(spTextured->u("tex"), 0);
	glDrawElements(GL_TRIANGLES, indicess.size(), GL_UNSIGNED_INT, indicess.data());
	// Clear vertex attribute state
	glDisableVertexAttribArray(spTextured->a("vertex"));  // Disable vertex attribute
	glDisableVertexAttribArray(spTextured->a("normal"));  // Disable normal attribute
	glDisableVertexAttribArray(spTextured->a("texCoord"));  // Disable normal attribute
}


//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float camera_x, float camera_y, float camera_z) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości
	
	glm::mat4 V = glm::lookAt(
		glm::vec3(camera_x, camera_y, camera_z),  // Pozycja kamery
		glm::vec3(0, 0, 0),  // Punkt patrzenia
		glm::vec3(0, 1, 0));  // Wektor wskazujący górę
	glm::mat4 P = glm::perspective(30.0f * PI / 180.0f, aspectRatio, 0.01f, 1200.0f); //Wylicz macierz rzutowania
	glm::mat4 M = glm::mat4(1.0f);

	kostka(P, V, M);
	testModel(P, V, M);

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

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

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

	//Główna pętla
	float camera_x = 0; 
	float camera_y = 0; 
	float camera_z = 0;
	glfwSetTime(0); //Zeruj timer
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		std::cout << camera_x << " " << camera_y << " " << camera_z << std::endl;
		camera_x += speed_x * glfwGetTime(); 
		camera_y += speed_y * glfwGetTime(); 
		camera_z += speed_z * glfwGetTime(); 
		glfwSetTime(0); //Zeruj timer
		drawScene(window, camera_x, camera_y, camera_z); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
