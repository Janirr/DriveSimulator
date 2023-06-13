#version 330

// Zmienne jednorodne
uniform sampler2D textureMap0; 
uniform sampler2D textureMap1;

//Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela
out vec4 pixelColor;

in vec2 iTexCoord0;
in vec2 iTexCoord1;
in vec4 ic; 

in vec4 n1;
in vec4 l1;
in vec4 v1;

in vec4 n2;
in vec4 l2;
in vec4 v2;

void main(void) {

	//Znormalizowane interpolowane wektory
	vec4 ml1 = normalize(l1);
	vec4 mn1 = normalize(n1);
	vec4 mv1 = normalize(v1);
	vec4 ml2 = normalize(l2);
	vec4 mn2 = normalize(n2);
	vec4 mv2 = normalize(v2);

	//Wektor odbity
	vec4 mr1 = reflect(-ml1, mn1);
	vec4 mr2 = reflect(-ml2, mn2);

	//Parametry powierzchni
	//kd = kolor pobierany z tekstur textureMap0 i textureMap1
	vec4 kd = mix(texture(textureMap0, iTexCoord0), texture(textureMap1, iTexCoord1),0.3); // mix tekstur 0 i 1 z wag¹ 0.3
	vec4 ks = vec4(1, 1, 1, 1); // kolor bia³y

	//Obliczenie modelu oœwietlenia
	float nl1 = clamp(dot(mn1, ml1), 0, 1); // iloczyn skalarno-wektorowe miêdzy wektorami normalnymi
	float rv1 = pow(clamp(dot(mr1, mv1), 0, 1), 50); // sk³adowa odbicia œwiat³a dla pierwszego Ÿród³a œwiat³a zgodnie z modelem oœwietlenia Phonga.
	float nl2 = clamp(dot(mn2, ml2), 0, 1); // iloczyn skalarno-wektorowe miêdzy wektorami normalnymi
	float rv2 = pow(clamp(dot(mr2, mv2), 0, 1), 50); // sk³adowa odbicia œwiat³a dla drugiego Ÿród³a œwiat³a zgodnie z modelem oœwietlenia Phonga.
	pixelColor=
	vec4(kd.rgb * nl1, kd.a) 
	+ vec4(ks.rgb*rv1, 0) 
	+ vec4(kd.rgb * nl2, kd.a) 
	+ vec4(ks.rgb*rv2, 0);
}
