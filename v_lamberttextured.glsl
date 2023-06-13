#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//Atrybuty
in vec4 vertex; //wspolrzedne wierzcholka w przestrzeni modelu
in vec4 color; //kolor zwi¹zany z wierzcho³kiem
in vec4 normal; //wektor normalny w przestrzeni modelu
in vec2 texCoord0;

//Zmienne interpolowane
out vec4 ic;
// swiatlo nr 1
out vec4 l1;
out vec4 n1;
out vec4 v1;
// swiatlo nr 2
out vec4 l2;
out vec4 n2;
out vec4 v2;
out vec2 iTexCoord0; 
out vec2 iTexCoord1;

void main(void) {
    vec4 lp1 = vec4(50, 30, 50, 1); //pozcyja œwiat³a, przestrzeñ œwiata
    l1 = normalize(V * lp1 - V*M*vertex); //wektor do œwiat³a w przestrzeni oka
    v1 = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //wektor do obserwatora w przestrzeni oka
    n1 = normalize(V * M * normal); //wektor normalny w przestrzeni oka

    vec4 lp2 = vec4(-40, 30, -50, 1); //pozcyja œwiat³a, przestrzeñ œwiata
    l2 = normalize(V * lp2 - V*M*vertex); //wektor do œwiat³a w przestrzeni oka
    v2 = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //wektor do obserwatora w przestrzeni oka
    n2 = normalize(V * M * normal); //wektor normalny w przestrzeni oka

    iTexCoord0 = texCoord0;
    iTexCoord1 = ((n1.xy + 1) + (n2.xy + 1))/4;

    ic = color;
    
    gl_Position=P*V*M*vertex; // transformacja wierzcho³ka do przestrzeni przyciêcia
}
