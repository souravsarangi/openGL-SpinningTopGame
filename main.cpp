/* Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* File for "Terrain" lesson of the OpenGL tutorial on
 * www.videotutorialsrock.com
 */

#include <time.h>
#include <string.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <cmath>

#endif

#define PI 3.14159265
#include "imageloader.h"
#include "vec3f.h"

using namespace std;

//Represents a terrain, by storing a set of heights and normals at 2D locations
class Terrain {
	private:
		int w; //Width
		int l; //Length
		float** hs; //Heights
		Vec3f** normals;
		bool computedNormals; //Whether normals is up-to-date
	public:
		Terrain(int w2, int l2) {
			w = w2;
			l = l2;
			
			hs = new float*[l];
			for(int i = 0; i < l; i++) {
				hs[i] = new float[w];
			}
			
			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}
			
			computedNormals = false;
		}
		
		~Terrain() {
			for(int i = 0; i < l; i++) {
				delete[] hs[i];
			}
			delete[] hs;
			
			for(int i = 0; i < l; i++) {
				delete[] normals[i];
			}
			delete[] normals;
		}
		
		int width() {
			return w;
		}
		
		int length() {
			return l;
		}
		
		//Sets the height at (x, z) to y
		void setHeight(int x, int z, float y) {
			hs[z][x] = y;
			computedNormals = false;
		}
		
		//Returns the height at (x, z)
		float getHeight(int x, int z) {
			return hs[z][x];
		}
		
		//Computes the normals, if they haven't been computed yet
		void computeNormals() {
			if (computedNormals) {
				return;
			}
			
			//Compute the rough version of the normals
			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}
			
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);
					
					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
					}
					
					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}
					
					normals2[z][x] = sum;
				}
			}
			
			//Smooth out the normals
			const float FALLOUT_RATIO = 0.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];
					
					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}
					
					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}
					normals[z][x] = sum;
				}
			}
			
			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;
			
			computedNormals = true;
		}
		
		//Returns the normal at (x, z)
		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};

//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for(int y = 0; y < image->height; y++) {
		for(int x = 0; x < image->width; x++) {
			unsigned char color =
				(unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}
	
	delete image;
	t->computeNormals();
	return t;
}

float _angle = -140.0f;
Terrain* _terrain;
float theta= 350.0f;
float yax=-3.0;
float xax=6.0;
float zax=6.0;
float fi=0.0;
float xpos=8.0;
float zpos=8.0;
float xvel=0.0;
float zvel=0.0;
float acc=0.0;
float tarx=55;
float tarz=rand()%30+15;
int cam=1;
float savy=0.0;
int score=30;

void cleanup() {
	delete _terrain;
}

void handleKeypress(unsigned char key, int x, int y) {
	//cout<<key<<endl;
	//cout<<xax<<" "<<yax<<" "<<zax<<" "<<theta<<" "<<_angle<<"\n";
	switch (key) {
		case 27: //Escape key
			cout<<xax<<" "<<yax<<" "<<zax<<" "<<theta<<" "<<_angle<<"\n";
			cleanup();
			exit(0);
		case 32:
		{
			theta+=10;
			break;
		}
		case 'w':
		{
			zax+=1;
			
			break;
		}
		case 's':{
		 	zax-=1;
		 	cout<<"hello\n";
		 	break;
		 }
		 case 'a':{
		 	xax+=1;
		 	break;
		 }
		 case 'd':{
		 	xax-=1;
		 	break;
		 }
		 case 'c':{
		 	_angle+=10;
		 	break;
		 }
		 case 'v':{
		 	_angle-=10;
		 	break;
		 }
		case 'z':
		{
			yax+=1;
			break;
		}
		case 'k':
		{	zpos+=1;
			break;
		}
		case 'h':
		{	zpos-=1;
			break;
		}
		case 'j':
		{	xpos-=1;
			break;
		}
		case 'u':
		{	xpos+=1;
			break;
		}
		case 'x':
		{	yax-=1;
			break;
		}
		case 'l':
		{

			xvel=acc*cos(fi);
			zvel=acc*sin(fi);
			acc=0;
			fi=0.0;
			break;
		}
		case '0':
		{
			glLoadIdentity();
			
			glRotatef(-theta, 1.0, 0.0, 0.0);
			glRotatef(-_angle, 0.0f, 1.0f, 0.0f);
			glTranslatef(0,yax,0);
			glTranslatef(xax,0,0);
			glTranslatef(0,0,zax);
			break;
		}
		case '1':
		{
			_angle = -140.0f;
			theta= 350.0f;
 			yax=-3.0;
			xax=6.0;
			zax=6.0;
			cam=1;
			break;
			
		}
		case '2':
		{
			xax=-1*xpos/5;
			zax=-1*zpos/5;
			yax=savy/5;
			theta=370;
			cam=2;
			break;

		}
		case '3':
		{
			xax=4,yax=-7,zax=4,theta=310,_angle=-140;
			cam=3;
			break;
		}
		case '5':
		{
			xax=-1*xpos/5-2;
			zax=-1*zpos/5-2;
			yax=savy/5-5;
			cam=5;
			theta=310,_angle=-140;
			break;
		}
	}
}


void handleKeypress2(int key, int x, int y) {
	//cout<<key<<endl;
	switch (key) {
		case GLUT_KEY_UP: //Escape key
		{
			acc+=0.3;
			break;
		}
		case GLUT_KEY_DOWN: //Escape key
		{
			acc-=0.3;
			break;
		}
		case GLUT_KEY_LEFT: //Escape key
		{
			fi-=0.05;
			break;
		}
		case GLUT_KEY_RIGHT: //Escape key
		{
			fi+=0.05;
			break;
		}
	}
}
void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}


float rot=0.0;

void drawtarget(){
	
	glPushMatrix();
	//float a = rand %60 ;

	glTranslatef(0.0,0.0, 12.0f);
	glRotatef(-90, 0, 1, 0);
    glColor3f(1.0, 0.0, 0.0);
    glutSolidTorus(0.8, 4.5, 25, 30);
    glColor3f(1.0, 1.0, 1.0);
    glutSolidTorus(0.8, 4, 25, 30);
    glColor3f(1.0, 0.0, 0.0);
    glutSolidTorus(0.8, 3, 25, 30);
    glColor3f(1.0, 1.0, 1.0);
    glutSolidTorus(0.8, 2, 25, 30);
    glColor3f(1.0, 0.0, 0.0);
    glutSolidTorus(0.8, 0.8, 25, 30);
    glPopMatrix();
 
}

void drawTop()
{
	glPushMatrix();
	
	glRotatef(rot,0,0,1.0);
	
	GLUquadricObj *quadratic;
	quadratic = gluNewQuadric();
	glPushMatrix();
	glTranslatef(0,0,0.5);
	glPushMatrix();
	glRotatef(180,1.0,0,0);
	glColor3f(0, 0, 1.0);
	glutWireCone(0.5f,0.5f,32,32);
	glPopMatrix();
	glPopMatrix();
	
	glTranslatef(0,0,0.5f);
	glColor3f(0, 1.0, 0);
	glutWireTorus(0.08,0.5,50,50);
	
	glTranslatef(0,0,0.12f);
	glColor3f(1.0, 1.0, 0);
	glutWireTorus(0.08,0.55,100,100);

	glTranslatef(0,0,0.12f);
	glColor3f(1.0, 0, 0);
	glutWireTorus(0.08,0.6,80,80);

	glTranslatef(0,0,-0.12f);
	glColor3f(0.65, 0.23, 0.23);
	gluCylinder(quadratic,0.1,0.1,1.0,80,80);

	rot+=1.0;
	glPopMatrix();
}
void RenderString(float x, float y, void *font, const char* string,float r,float g,float b,int rev)
{  

  glColor3f(r, g, b); 
  glRasterPos3f(x, y,0);
  int xx = strlen(string);
  if(rev==0)
  for (int i=0;i<xx;i++)
  glutBitmapCharacter(font, string[i]);
else if(rev==1)
	for (int i=xx-1;i>=0;i--)
  glutBitmapCharacter(font, string[i]);
}

void calcScore()
{
    int s=score;
    char str[80];
    int p=s,i=0;
    while(p){
    	str[i++]=p%10+'0';p/=10;
    }
    str[i]='0';
    str[i+1]=0;


    //str=strrev(str);
    glBegin(GL_QUADS);
        glVertex2f(-5.0, 4.0);
        glVertex2f(-3.5, 4.0);
        glVertex2f(-3.5, 2.0);
        glVertex2f(-5.0, 2.0);
    glEnd();
    RenderString(-4.5,3.0, GLUT_BITMAP_TIMES_ROMAN_24, "Score: ", 95.0,0.2705,0.0,0);
    glTranslatef(-100,0,0);
    RenderString(-3.8,3.0, GLUT_BITMAP_TIMES_ROMAN_24, str,100.0f, 1.0f, 0.0f,1);
}
void detect()
{
	//cout<<xpos<<" "<<tarx<<" "<<zpos<<" "<<tarz<<endl;
	
	if(sqrt((xpos-tarx)*(xpos-tarx)+(zpos-tarz-22)*(zpos-tarz-22))<7.5)
	{
	score+=1;
    _angle = -140.0f;
	theta= 350.0f;
	yax=-3.0;
 	xax=6.0;
 	zax=6.0;
 	fi=0.0;
 	xpos=8.0;
 	zpos=8.0;
 	xvel=0.0;
 	zvel=0.0;
	 acc=0.0;
 	tarx=55;

	 tarz=rand()%30+15;
	cam=1;
	 savy=0.0;
	}
}
void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	
	
	glPushMatrix();
	glTranslatef(60,0,60);
	glScalef(0.1,0.1,0.1);
	calcScore();
	glPopMatrix();
	

	handleKeypress('0',0,0);


	glPushMatrix();
	GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
	
	GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	
	float scale = 5.0f / max(_terrain->width() - 1, _terrain->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef(-(float)(_terrain->width() - 1) / 2,
				 0.0f,
				 -(float)(_terrain->length() - 1) / 2);

	
	glPushMatrix();


	glTranslatef(xpos,_terrain->getHeight((int)xpos,(int)zpos),zpos);
	savy=_terrain->getHeight((int)xpos,(int)zpos)+3;
	if(cam==2)
	{
		xax=-1*xpos/5+1;
		zax=-1*zpos/5+1;
		yax=-1*savy/5-1;
		
	}
	if(cam==5)
	{
		xax=-1*xpos/5+8;
		zax=-1*zpos/5+8;
		yax=savy/5-3;
		
	}
	/*if((_terrain->getHeight((int)xpos,(int)zpos)-_terrain->getHeight((int)(xpos+xvel),(int)(zpos+zvel)))>0.0)
	{
			xvel-=0.5;
			zvel-=0.5;
	}
	if((_terrain->getHeight((int)xpos,(int)zpos)-_terrain->getHeight((int)(xpos+xvel),(int)(zpos+zvel)))<0.0)
	{
			xvel+=0.5;
			zvel+=0.5;
	}
*/

	glBegin(GL_LINES);
	glColor3f(0,0.7,1);
    glVertex2f(0,0);
    glVertex2f(60*cos(fi),60*sin(fi));
	glEnd();

	//cout<<xvel<<" "<<zvel<<endl;
	glRotatef(-90,1.0,0,0);
	glScalef(5, 5, 5);
	Vec3f normal = _terrain->getNormal(xpos, zpos);
	Vec3f vertical=Vec3f(0.0,1.0,0.0);
	Vec3f perp=vertical.cross(normal).normalize();
	glRotatef(acos(vertical.dot(normal)/(normal.magnitude()*vertical.magnitude()))*180.0/PI,perp.v[0],perp.v[1],perp.v[2]);


 	drawTop();
	glPopMatrix();
	
	glPushMatrix();
	glColor3f(0.69f, 0.3f, 0.2f);
	for(int z = 0; z < _terrain->length() - 1; z++) {
		
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < _terrain->width(); x++) {
			Vec3f normal = _terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _terrain->getHeight(x, z), z);
			normal = _terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}
	glPopMatrix();

	glPushMatrix();
	glTranslatef(tarx,0,tarz);
	//cout<<-1*_terrain->width()<<" "<<-1*_terrain->length()<<endl;
	glScalef(2,2,2);

	drawtarget();
	glPopMatrix();
	
	glPopMatrix();
	detect();
	glutSwapBuffers();
}

void update(int value) {
	//_angle += 1.0f;
	
	xpos+=xvel;
	zpos+=zvel;
	if (xvel>-0.08&&xvel<0.08)
	{
		xvel=0.0;
	}
	if (zvel>-0.08&&zvel<0.08)
	{
		zvel=0.0;
	}
	if(xvel>0.0)
		xvel-=0.05;
	else if(xvel<0)
		xvel+=0.05;
	if(zvel>0.0)
		zvel-=0.05;
	else if(zvel<0)
		zvel+=0.05;
	if (_angle > 360) {
		_angle -= 360;
	}
	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}



int main(int argc, char** argv) {
	time_t t;
   srand((unsigned) time(&t));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	
	glutCreateWindow("Assignment 2");
	initRendering();
	
	_terrain = loadTerrain("heightmap.bmp", 20);
	
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(handleKeypress2);
	glutReshapeFunc(handleResize);
	glutTimerFunc(25, update, 0);
	
	glutMainLoop();
	return 0;
}









