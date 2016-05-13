#define FREEGLUT_STATIC
#include <cstdio>	
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <Windows.h>
#include <mmsystem.h>   //导入声音头的文件	 
#include <GL/glut.h>
#include <GL/GLAUX.H>
#include <GL/GL.h>
#include <GL/GLU.h>

#pragma comment(lib,"winmm.lib")	//导入声音头的文件库 
#pragma comment(lib,"glaux.lib")

using namespace std;

#define room_h  1.0f
#define IDM_APPLICATION_EXIT    (101)
#define IDM_APPLICATION_TEXTURE (102)
#define IDM_APPLICATION_BANK    (103)
#define MAZE_HEIGHT (16)
#define MAZE_WIDTH  (16)
#define STARTING_POINT_X (13.5f);
#define STARTING_POINT_Y (1.5f);
#define STARTING_HEADING (90.0f);
#define window_size_x 800
#define window_size_y 800
#define MAX_rotate_rate 0.05f
#define num_of_pic 2

GLuint g_texture[num_of_pic];  //To store the texture

float player_x = STARTING_POINT_X;
float player_y = STARTING_POINT_Y;
float player_h = STARTING_HEADING;   //视角朝向
float player_s = 0.0f;  // 前进速度
float player_l = 0.0f;  // 左走速度 
float player_m = 1.5f;  // 上帝视角下的速度是普通视角的player_m倍
float player_t = 0.0f;  // 视角旋转
float player_b = 0.0f;  // viewpoint bank (roll)
static float texcoordX = 0.0f;

int walllist = 0;
int mazelist = 0;
int balllist = 0;
int status = 1;
bool keystate[4] = { false, false, false, false };

char mazedata[MAZE_HEIGHT][MAZE_WIDTH] = {
	{ 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H' },
	{ 'H', ' ', 'H', ' ', ' ', ' ', ' ', ' ', 'H', ' ', ' ', ' ', ' ', ' ', ' ', 'H' },
	{ 'H', ' ', 'H', ' ', 'H', 'H', 'H', ' ', 'H', ' ', 'H', ' ', ' ', ' ', ' ', 'H' },
	{ 'H', ' ', 'H', 'H', ' ', ' ', 'H', ' ', 'H', 'H', ' ', 'H', ' ', 'H', ' ', 'H' },
	{ 'H', ' ', ' ', ' ', ' ', ' ', 'H', ' ', ' ', ' ', ' ', ' ', ' ', 'H', ' ', 'H' },
	{ 'H', ' ', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', ' ', 'H', 'H', 'H', ' ', 'H' },
	{ 'H', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'H', ' ', ' ', 'H' },
	{ 'H', ' ', 'H', 'H', 'H', 'H', 'H', ' ', 'H', 'H', 'H', ' ', 'H', 'H', 'H', 'H' },
	{ 'H', ' ', 'H', ' ', ' ', ' ', 'H', ' ', ' ', ' ', 'H', ' ', ' ', ' ', ' ', 'H' },
	{ 'H', ' ', ' ', ' ', 'H', 'H', 'H', 'H', 'H', 'H', 'H', ' ', ' ', ' ', ' ', 'H' },
	{ 'H', ' ', 'H', ' ', ' ', ' ', 'H', ' ', ' ', ' ', 'H', ' ', ' ', 'H', ' ', 'H' },
	{ 'H', ' ', 'H', 'H', 'H', 'H', 'H', ' ', 'H', 'H', 'H', 'H', ' ', 'H', ' ', 'H' },
	{ 'H', ' ', ' ', ' ', ' ', ' ', 'H', ' ', ' ', ' ', ' ', ' ', ' ', 'H', ' ', 'H' },
	{ 'H', ' ', ' ', 'H', 'H', ' ', 'H', 'H', 'H', 'H', ' ', 'H', 'H', 'H', ' ', 'H' },
	{ 'H', ' ', ' ', ' ', 'H', ' ', 'H', ' ', ' ', ' ', ' ', 'H', ' ', ' ', ' ', 'H' },
	{ 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', ' ', 'H', 'H' },
};

void PlayMusic();
AUX_RGBImageRec *LoadBMP(char * Filename);
int LoadGLTextures(GLuint& unTexture, const char* chFileName);
void move_forward(float player_s, float bf);
void move_left(float player_l);
void move_control(int key, int x, int y);
void change_direction(int x, int y);
void myinit();
bool wall(int x, int y);
bool onopen(int x, int y);
void closeit(int x, int y);
bool neighbor(int x, int y, int w, int *nx, int *ny);
bool diagnal(int x, int y, int w, int *nx, int *ny);
void dw(int x, int y, int p);
void drawwalls(void);
void drawtop(void);
void drawball(void);
void navmaze1();
void navmaze2();
void myDisplay();
void myReshape(int w, int h);
void upSpecialKeyboard(int key, int x, int y);
void idle();

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(window_size_x, window_size_y);
	glutCreateWindow("Maze Runner —— Zhoujunpei 3140102286 Control by ↑ ↓ → ← and mouse, press F1 and F2 to change the view");
	glutDisplayFunc(myDisplay);
	myinit();
	glutSpecialFunc(move_control);
	glutPassiveMotionFunc(change_direction);
	glutSpecialUpFunc(upSpecialKeyboard);
	glutIdleFunc(idle);
	glutMainLoop();
}

//用于播放音乐的函数，在myinit中调用
void PlayMusic()
{
	char buf[128];
	char str[128] = { 0 };
	int i = 0;
	MCI_OPEN_PARMS mciOpen;
	MCIERROR mciError;
	mciOpen.lpstrDeviceType = "mpegvideo";
	mciOpen.lpstrElementName = "FortunateSon.mp3";
	mciError = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mciOpen);
	if (mciError) {
		mciGetErrorString(mciError, buf, 128);
		printf("send MCI_OPEN command failed:%s\n", buf);
		return;
	}
	UINT DeviceID = mciOpen.wDeviceID;
	MCI_PLAY_PARMS mciPlay;
	mciError = mciSendCommand(DeviceID, MCI_PLAY, 0, (DWORD)&mciPlay);
	if (mciError) {
		printf("send MCI_PLAY command failed\n");
		return;
	}
}

//用于载入位图图象
AUX_RGBImageRec *LoadBMP(char * Filename)  
{
	FILE *File = NULL;
	if (!Filename)
		return NULL;
	File = fopen(Filename, "r");
	if (File)
	{
		fclose(File);
		return auxDIBImageLoadA(Filename);
	}
	return NULL;
}

// 载入位图(调用上面的代码)并转换成纹理 
int LoadGLTextures(GLuint& unTexture, const char* chFileName)
{
	int Status = FALSE; // Status 状态指示器  
	AUX_RGBImageRec *TextureImage;	//保存贴图数据的指针
	char* file_name = (char*)chFileName;
	if (TextureImage = LoadBMP(file_name))	//载入贴图数据
	{
		Status = 1;
		//创建一个纹理，unTexture
		glGenTextures(1, &unTexture);
		//绑定纹理，然后对该纹理区添加纹理数据
		glBindTexture(GL_TEXTURE_2D, unTexture);   
		// 生成纹理 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage->sizeX, TextureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
		//设置滤波为线性滤波
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
		//线性滤波
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
		//释放资源
		if (TextureImage)													
		{
			if (TextureImage->data)
			{
				free(TextureImage->data);
			}
			free(TextureImage);
		}
	}
	return Status;
}

void myinit()
{
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	walllist = glGenLists(2);    //显示列表是把一组opengl语句预先存放起来，方便调用
	mazelist = walllist + 1;
	balllist = walllist + 2;
	glNewList(walllist, GL_COMPILE);   //标志一个显示列表的开始，GL_COMPILE参数说明后面的语句只存入不执行
	drawwalls();
	glEndList();
	glNewList(mazelist, GL_COMPILE);
	drawtop();
	glEndList();
	glNewList(balllist, GL_COMPILE);
	drawball();
	glEndList();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 0.1, 60.0);
	glMatrixMode(GL_MODELVIEW);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);//使纹理不变形
	PlayMusic();
	LoadGLTextures(g_texture[0], "sky.bmp");
}

bool wall(int x, int y)
{
	return (x >= 0 && y >= 0 && x<MAZE_WIDTH && y<MAZE_HEIGHT && mazedata[y][x] != ' ');
}

bool onopen(int x, int y)
{
	if (wall(x, y)){
		return(mazedata[y][x] == 'H');
	}
}

void closeit(int x, int y) 
{
	if (onopen(x, y))
	{
		mazedata[y][x] = 'X';
	}
}

bool neighbor(int x, int y, int w, int *nx, int *ny)
{
	switch (w) {
	case 0:
		*nx = x - 1; *ny = y;   break;
	case 1:
		*nx = x;   *ny = y + 1; break;
	case 2:
		*nx = x + 1; *ny = y;   break;
	case 3:
		*nx = x;   *ny = y - 1; break;
	default:
		break;
	}
	return wall(*nx, *ny);
}

bool diagnal(int x, int y, int w, int *nx, int *ny)
{
	switch (w) {
	case 0:
		*nx = x - 1; *ny = y - 1; break;
	case 1:
		*nx = x - 1; *ny = y + 1; break;
	case 2:
		*nx = x + 1; *ny = y + 1; break;
	case 3:
		*nx = x + 1; *ny = y - 1; break;
	default:
		break;
	}
	return wall(*nx, *ny);
}

void dw(int x, int y, int p) 
{
	int w = p;
	closeit(x, y);
	do{
		int x2 = 0;
		int y2 = 0;
		if (neighbor(x, y, w, &x2, &y2)){
			if (onopen(x2, y2)) 
				dw(x2, y2, (w + 3) % 4);
			else if ((w + 1) % 4 == p)
					return;
		}
		else {
			float fx;
			float fy;
			if (diagnal(x, y, w, &x2, &y2) && onopen(x2, y2)) {
				dw(x2, y2, (w + 2) % 4);
			}
			texcoordX = (texcoordX<0.5) ? 1.0f : 0.0f;
			fx = (float)x + ((w == 1 || w == 2) ? 1.0f : 0.0f);
			fy = (float)y + ((w == 0 || w == 1) ? 1.0f : 0.0f);
			glTexCoord2f(texcoordX, 0.0f);
			glVertex3f(fx, fy, 0.0f);
			glTexCoord2f(texcoordX, 1.0f);
			glVertex3f(fx, fy, 1.0f);
		}
		w++; w %= 4;
	} while (w != p);
	return;
}

void drawwalls() 
{
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUAD_STRIP);
	glColor3f(1.0, 1.0, 1.0);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1.0f);
	dw(0, 0, 0);
	glEnd();
}

void drawtop()
{
	int x, y;
	glBegin(GL_QUADS);
	for (y = 0; y<MAZE_HEIGHT; y++) {
		for (x = 0; x<MAZE_WIDTH; x++) {
			if (wall(x, y)) {
				mazedata[y][x] = 'X';
				glVertex3f(x + 0.0f, y + 0.0f, 1.0f);
				glVertex3f(x + 1.0f, y + 0.0f, 1.0f);
				glVertex3f(x + 1.0f, y + 1.0f, 1.0f);
				glVertex3f(x + 0.0f, y + 1.0f, 1.0f);
			}
		}
	}
	glEnd();
}

void move_forward(float player_s, float bf)
{
	float px = player_x + player_s*(float)sin(player_h*3.14 / 180);
	float py = player_y + player_s*(float)cos(player_h*3.14 / 180);
	int x = ((int)player_x);
	int y = ((int)player_y);
	if ((px> x + 1.0f - bf) && wall(x + 1, y)) 
		px = (float)(x)+1.0f - bf;
	if (py> y + 1.0f - bf && wall(x, y + 1))  
		py = (float)(y)+1.0f - bf;
	if (px< x + bf && wall(x - 1, y))
		px = (float)(x)+bf;
	if (py< y + bf && wall(x, y - 1)) 
		py = (float)(y)+bf;
	player_x = px;
	player_y = py;
}

void move_left(float player_l)
{
	float target_x = player_x - player_l*(float)cos(player_h*3.14 / 180);
	float target_y = player_y + player_l*(float)sin(player_h*3.14 / 180);
	int x = (int)target_x;
	int y = (int)target_y;
	if(!wall(x, y)){
		player_x = target_x;
		player_y = target_y;
	}
}

void drawball()
{
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.0, 1.0, 0.0);
	glutSolidSphere(0.2f, 15, 15);
}

void navmaze1()
{
	move_forward(player_s, 0.2f);
	move_left(player_l);
	player_h += player_t;
	player_b = 3 * player_b / 4 + player_t / 4;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glPushMatrix();
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(player_h, 0.0f, 0.0f, 1.0f);
	glTranslatef(-player_x, -player_y, -0.5f);
	glCallList(walllist);
	glPopMatrix();
}

void navmaze2()
{
	move_forward(player_m*player_s, 0.2f);
	move_left(player_m*player_l);
	player_h += player_t;
	player_b = 3 * player_b / 4 + player_t / 4;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glOrtho(-16.0, 16.0, -16.0, 16.0, -2.0, 20.0);
	glPushMatrix();
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(-MAZE_WIDTH / 2, -MAZE_HEIGHT / 2, -0.5f);
	glCallList(walllist);
	glCallList(mazelist);
	glPushMatrix();
	glTranslatef(player_x, player_y, 0.5f);
	glCallList(balllist);
	glPopMatrix();
	glPopMatrix();
}

void myDisplay()
{
	if (status == 1)
		navmaze1();
	else if (status == 2)
		navmaze2();
	glFlush();
	glutSwapBuffers();
}

void myReshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutPostRedisplay();
}

void move_control(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		keystate[2] = true;
		player_l = 0.001f;
		break;
	case GLUT_KEY_RIGHT:
		keystate[3] = true;
		player_l = -0.001f;
		break;
	case GLUT_KEY_UP:
		keystate[0] = true;
		player_s = 0.001f;
		break;
	case GLUT_KEY_DOWN:
		keystate[1] = true;
		player_s = -0.001f;
		break;
	case GLUT_KEY_F1:
		status = 1;
		glutPostRedisplay();
		break;
	case GLUT_KEY_F2:
		status = 2;
		glutPostRedisplay();
		break;
	default:break;
	}
}

void change_direction(int x, int y)
{
	float left_bound = window_size_x / 3.0;
	float right_bound = window_size_x / 3.0*2.0;
	if (x < left_bound){
		keystate[2] = true;
		player_t = -(MAX_rotate_rate - MAX_rotate_rate / left_bound * x);
	}
	else if (x > right_bound){
		keystate[3] = true;
		player_t = (x - right_bound)*MAX_rotate_rate / (window_size_x - right_bound);
	}
	else
		player_t = 0.0f;
}

//按键没有被按下的时候
void upSpecialKeyboard(int key,int x,int y)
{
	switch (key)
	{
		case GLUT_KEY_LEFT:
			keystate[2] = false;
			player_l = 0.0f;
			break;
		case GLUT_KEY_RIGHT:
			keystate[3] = false;
			player_l = 0.0f;
			break;
		case GLUT_KEY_UP:
			keystate[0] = false;
			player_s = 0.0f;
			break;
		case GLUT_KEY_DOWN:
			keystate[1] = false;
			player_s = 0.0f;
			break;
		default:break;
	}
//	glutPostRedisplay();
}

//释放按键后就进入空闲状态，如果空闲状态一直重复绘图开销很大
void idle()
{
	if (keystate[0] || keystate[1] || keystate[2] || keystate[3]) glutPostRedisplay();
	else{}
}