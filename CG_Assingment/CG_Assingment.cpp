// CG_Assingment.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<fstream>
#include<vector>
#include<string>

#include <GL/glew.h>
#include <GL/glut.h>

using namespace std;

class Vertices{
private:
	static Vertices* instance;
	Vertices(){};

public:
	~Vertices();
	static Vertices* getInstance()
	{
		if (instance == NULL)
			instance = new Vertices();
		return instance;
	}
	GLuint* vertexBuff;
	GLuint* vertexArr;

	vector<GLfloat> points; //vertex�� //ù��° 5ĭ�� ���� �̸��������� ��.
	vector<GLfloat> lines;	//line�� 
	vector<GLfloat> lines_strip; //line strip�� 

	int primitives; //���� ��� �׸�����. 0:pint, 1:line, 2:line_strip

	int count; //��object�� �̷�� ���� ����.
	int start; //�������� �ε�����

	bool color_mode; //�� �ٲٴ� ��� TRUE : ��ü �� �ٲٱ�, FALSE : ���� �ֱٿ� �׸��ͺ��� ���������� ������ �׷ȴ��� ����.
	int selected; //���õ� object�� order������ index��.

	vector <int> order; // object���� �׷��� ����->�� �࿡ 3�����迭: primitive�� ����, ���� �ε��� ��ȣ, ���� ����
};
Vertices* Vertices::instance;

int g_screenWidth, g_screenHeight;
GLuint g_programID;

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
	//create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLint Result = GL_FALSE;
	int InfoLogLength;

	//Read the vertex shader code from the file
	string VertexShaderCode;
	ifstream VertexShaderStream(vertex_file_path, ios::in);
	if (VertexShaderStream.is_open())
	{
		string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	//Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	//Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	//Read the fragment shader code from the file
	string FragmentShaderCode;
	ifstream FragmentShaderStream(fragment_file_path, ios::in);
	if (FragmentShaderStream.is_open())
	{
		string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	//Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	//Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	//Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	vector<char> ProgramErrorMessage(InfoLogLength);
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void renderScene(void)
{
	//Clear all pixels
	glClear(GL_COLOR_BUFFER_BIT);

	//���׸���
	glBindVertexArray(Vertices::getInstance()->vertexArr[0]);
	glDrawArrays(GL_POINTS, 0, Vertices::getInstance()->points.size()/5);

	//���׸���
	glBindVertexArray(Vertices::getInstance()->vertexArr[1]);
	glDrawArrays(GL_LINES, 0, Vertices::getInstance()->lines.size()/5);
	
	//����� �� �׸���
	for (int i = 0; i < Vertices::getInstance()->order.size(); i+=3)
	{
		if (Vertices::getInstance()->order[i] == 2)
		{
			int start = Vertices::getInstance()->order[i+1];//�������� �ε�����
			int num = Vertices::getInstance()->order[i+2];//���� ����
			glBindVertexArray(Vertices::getInstance()->vertexArr[2]);
			glDrawArrays(GL_LINE_STRIP, start/5, num);
		}
	}

	//Double buffer
	glutSwapBuffers();
}

void init()
{
	//initilize the glew and check the errors.
	GLenum res = glewInit();
	if (res != GLEW_OK)
	{
		fprintf(stderr, "Error: '%s' \n", glewGetErrorString(res));
	}

	//select the background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	//�� ������ �ʱ�ȭ.
	Vertices::getInstance()->primitives = 0;
	Vertices::getInstance()->count = 0;
	Vertices::getInstance()->start = 0;
	Vertices::getInstance()->color_mode = false; 
	
	//���� �̸������� ���� �ʱ�ȭ �Ѵ�.
	Vertices::getInstance()->points.push_back(0.95);
	Vertices::getInstance()->points.push_back(0.95);
	Vertices::getInstance()->points.push_back(0.0);
	Vertices::getInstance()->points.push_back(0.0);
	Vertices::getInstance()->points.push_back(0.0);
}

void myMouse(int button, int state, int x, int y)//���� �Ķ���ʹ� �� �̰͵��� ���ߵǿ�
{
	float fx, fy;
	fx = x / (float)(g_screenWidth - 1)*2.0f - 1.0f;//x�� ��ġ �޴¹�
	fy = -(y / (float)(g_screenHeight - 1)*2.0f - 1.0f);//y����ġ �޴¹�. (��ġ�� �޾ƿ����� ����� �׸��׸���ǰ���?)

	
	GLfloat color[3];
	color[0] = Vertices::getInstance()->points[2];
	color[1] = Vertices::getInstance()->points[3];
	color[2] = Vertices::getInstance()->points[4];

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (Vertices::getInstance()->primitives == 0) //���� �׸��� ����϶�
		{
			Vertices::getInstance()->points.push_back(fx);
			Vertices::getInstance()->points.push_back(fy);
			Vertices::getInstance()->points.push_back(color[0]);
			Vertices::getInstance()->points.push_back(color[1]);
			Vertices::getInstance()->points.push_back(color[2]);
			glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->points.size(), Vertices::getInstance()->points.data(), GL_STATIC_DRAW);
			Vertices::getInstance()->order.push_back(0); //� primitive�� ������
			Vertices::getInstance()->order.push_back(Vertices::getInstance()->points.size()-5);//���° ������.
			Vertices::getInstance()->order.push_back(1);//primitive�� ���° ������.
			
			Vertices::getInstance()->selected = Vertices::getInstance()->order.size()-3;
		}
		else if (Vertices::getInstance()->primitives == 1) //���� �׸��� ����϶�
		{
			Vertices::getInstance()->lines.push_back(fx);
			Vertices::getInstance()->lines.push_back(fy);
			Vertices::getInstance()->lines.push_back(color[0]);
			Vertices::getInstance()->lines.push_back(color[1]);
			Vertices::getInstance()->lines.push_back(color[2]);
			
			glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->lines.size(), Vertices::getInstance()->lines.data(), GL_STATIC_DRAW);
			
			
			if (Vertices::getInstance()->count == 0)//���� ù��° ���� �������
			{
				Vertices::getInstance()->count = 1;
				Vertices::getInstance()->start = Vertices::getInstance()->lines.size() - 5; //������������ ���� ������
			}
			else//���� �ι�° ���� �������
			{
				Vertices::getInstance()->count = 0;	
				int start = Vertices::getInstance()->start;

				Vertices::getInstance()->order.push_back(1);
				Vertices::getInstance()->order.push_back(start);
				Vertices::getInstance()->order.push_back(2);
				
				Vertices::getInstance()->start = 0; 
				Vertices::getInstance()->selected = Vertices::getInstance()->order.size() - 3;
			}
		}
		else if (Vertices::getInstance()->primitives == 2) //line strip�� �׸���
		{
			Vertices::getInstance()->lines_strip.push_back(fx);
			Vertices::getInstance()->lines_strip.push_back(fy);
			Vertices::getInstance()->lines_strip.push_back(color[0]);
			Vertices::getInstance()->lines_strip.push_back(color[1]);
			Vertices::getInstance()->lines_strip.push_back(color[2]);
		
			glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->lines_strip.size() , Vertices::getInstance()->lines_strip.data(), GL_STATIC_DRAW);
		
			if (Vertices::getInstance()->count == 0)//���� ù��° ���� �������
			{
				Vertices::getInstance()->count = 1;
				Vertices::getInstance()->start = Vertices::getInstance()->lines_strip.size()-5; //������������ ���� �������� �ε���
			}
			else
			{
				Vertices::getInstance()->count++;
				int start = Vertices::getInstance()->start;
				int num = Vertices::getInstance()->count;
				if (num == 2)
				{
					Vertices::getInstance()->order.push_back(2);
					Vertices::getInstance()->order.push_back(start);
					Vertices::getInstance()->order.push_back(2);

					Vertices::getInstance()->selected = Vertices::getInstance()->order.size() - 3;
				}
				Vertices::getInstance()->order[Vertices::getInstance()->order.size()-1]=num; //object�� �����ϴ� ���� ������ update�Ѵ�
			}
		}
	}
	//���ٲٱ�.
	else if (button == GLUT_RIGHT_BUTTON&&state == GLUT_DOWN)
	{
		bool mode = Vertices::getInstance()->color_mode;
		GLfloat r = Vertices::getInstance()->points[2]; //�̸������� ����
		GLfloat g = Vertices::getInstance()->points[3]; //�̸������� �Ķ�
		GLfloat b = Vertices::getInstance()->points[4]; //�̸������� �ʷ�
		//��ü���ٲٱ�
		if (mode) 
		{
			//point
			for (int i = 5; i < Vertices::getInstance()->points.size(); i++) //point[1~5]�� �� �̸����� ��.
			{
				if (i % 5 == 2)
					Vertices::getInstance()->points[i] = r;
				else if (i % 5 == 3)
					Vertices::getInstance()->points[i] = g;
				else if (i % 5 == 4)
					Vertices::getInstance()->points[i] = b;
			}
			glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->points.size(), Vertices::getInstance()->points.data(), GL_STATIC_DRAW);
			//line
			for (int i = 0; i < Vertices::getInstance()->lines.size(); i++)
			{
				if (i % 5 == 2)
					Vertices::getInstance()->lines[i] = r;
				else if (i % 5 == 3)
					Vertices::getInstance()->lines[i] = g;
				else if (i % 5 == 4)
					Vertices::getInstance()->lines[i] = b;
			}
			glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->lines.size(), Vertices::getInstance()->lines.data(), GL_STATIC_DRAW);	
			//line_strip
			for (int i = 0; i < Vertices::getInstance()->lines_strip.size(); i++) 
			{
				if (i % 5 == 2)
					Vertices::getInstance()->lines_strip[i] = r;
				else if (i % 5 == 3)
					Vertices::getInstance()->lines_strip[i] = g;
				else if (i % 5 == 4)
					Vertices::getInstance()->lines_strip[i] = b;
			}
			glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->lines_strip.size(), Vertices::getInstance()->lines_strip.data(), GL_STATIC_DRAW);
		}

		//�ϳ��� �� �ٲٱ�.
		else
		{
			int selection = Vertices::getInstance()->selected;
			if (Vertices::getInstance()->order[selection] == 0)
			{
				Vertices::getInstance()->points[Vertices::getInstance()->order[selection+1] + 2] = r;
				Vertices::getInstance()->points[Vertices::getInstance()->order[selection+1] + 3] = g;
				Vertices::getInstance()->points[Vertices::getInstance()->order[selection+1] + 4] = b;
				glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[0]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->points.size(), Vertices::getInstance()->points.data(), GL_STATIC_DRAW);
			}
			else if (Vertices::getInstance()->order[selection] == 1 ) // ���̰� �� ���� �ϼ������� ���� �̷�� �� ���� ���� �ٲ۴�.
			{
				Vertices::getInstance()->lines[Vertices::getInstance()->order[selection + 1] + 2] = r;
				Vertices::getInstance()->lines[Vertices::getInstance()->order[selection + 1] + 3] = g;
				Vertices::getInstance()->lines[Vertices::getInstance()->order[selection + 1] + 4] = b;
				glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->lines.size(), Vertices::getInstance()->lines.data(), GL_STATIC_DRAW);

				Vertices::getInstance()->lines[Vertices::getInstance()->order[selection + 1] + 7] = r;
				Vertices::getInstance()->lines[Vertices::getInstance()->order[selection + 1] + 8] = g;
				Vertices::getInstance()->lines[Vertices::getInstance()->order[selection + 1] + 9] = b;

				glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->lines.size(), Vertices::getInstance()->lines.data(), GL_STATIC_DRAW);
			}
			else if (Vertices::getInstance()->order[selection] == 2) //line strip�� �̷�� ������ ���� �ٲ۴�.
			{
				for (int i = 0; i < Vertices::getInstance()->order[selection + 2]; i++)
				{
					Vertices::getInstance()->lines_strip[Vertices::getInstance()->order[selection + 1] + 2 + i*5] = r;
					Vertices::getInstance()->lines_strip[Vertices::getInstance()->order[selection + 1] + 3 + i*5] = g;
					Vertices::getInstance()->lines_strip[Vertices::getInstance()->order[selection + 1] + 4 + i*5] = b;

					glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[2]);
					glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->lines_strip.size(), Vertices::getInstance()->lines_strip.data(), GL_STATIC_DRAW);
				}
				
			}
		}
	}
}

void myKeyboard(unsigned char key, int x, int y)
{
	int primitives = Vertices::getInstance()->primitives;
	int count = Vertices::getInstance()->count;

	if (key == 'm') //��带 �ٲٴ� Ű
	{
		if (primitives == 1&&Vertices::getInstance()->lines.size()%2==1) //������ �ϼ����� �ʾ����� ������.
		{
			for (int i = 0; i < 5; i++)
			{
				Vertices::getInstance()->lines.pop_back();
			}
		}
		else if (primitives == 2 ) 
		{
			if (count == 1)//������ �ϼ����� �ʾ����� ������(�� �ϳ��� �������)
			{
				for (int i = 0; i < 5; i++)
				{
					Vertices::getInstance()->lines_strip.pop_back();
				}
			}
		}
		if (primitives < 2)
			Vertices::getInstance()->primitives++;
		else if (primitives == 2)
			Vertices::getInstance()->primitives = 0;
		//�ʱ�ȭ.
		Vertices::getInstance()->start = 0;
		
		Vertices::getInstance()->count = 0;
	}

	//�̸����� ���ٲٱ�.
	else if (key == 'r')
	{
		if (Vertices::getInstance()->points[2] < 1.0)
			Vertices::getInstance()->points[2] += 0.1;
		else
			Vertices::getInstance()->points[2] = 0.0;
	}
	else if (key == 'g')
	{
		if (Vertices::getInstance()->points[3] < 1.0)
			Vertices::getInstance()->points[3] += 0.1;
		else
			Vertices::getInstance()->points[3] = 0.0;
	}
	else if (key == 'b')
	{
		if (Vertices::getInstance()->points[4] < 1.0)
			Vertices::getInstance()->points[4] += 0.1;
		else
			Vertices::getInstance()->points[4] = 0.0;
	}

	//�̸����� ������ ��ü �� �ٲٱ�.
	else if (key == 'u')
	{
		Vertices::getInstance()->color_mode = TRUE;
	}
	//�̸����� ������ �ϳ��� �ٲٱ�.
	else if (key == 'e')
	{
		if (Vertices::getInstance()->color_mode == TRUE)
		{
			Vertices::getInstance()->color_mode = FALSE;
		}
		else
		{
			if (Vertices::getInstance()->selected - 3>0)
				Vertices::getInstance()->selected -= 3;
			else
				Vertices::getInstance()->selected = Vertices::getInstance()->order.size() / 3 - 1;
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->points.size(), Vertices::getInstance()->points.data(), GL_STATIC_DRAW);
	glutPostRedisplay();
}
int main(int argc, char**argv){

	//init GLUT and create Window
	//initialize the GLUT
	glutInit(&argc, argv);
	//GLUT_DOUBLE enables double buffering (drawing to a background buffer while the other buffer is displayed)
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	//These two functions are used to define the position and size of the window. 
	glutInitWindowPosition(200, 200);

	g_screenWidth = 700;
	g_screenHeight = 400;
	glutInitWindowSize(g_screenWidth, g_screenHeight);
	
	//This is used to define the name of the window.
	glutCreateWindow("201411237 ������- Assignment#1");

	//call initization function
	init();

	//���̴��� ID�ޱ�
	GLuint programID = LoadShaders("VertexShader.txt", "FragmentShader.txt");
	glUseProgram(programID);
	g_programID = programID;

	GLint posID = glGetAttribLocation(g_programID, "position"); //��ġ
	GLint colorID = glGetAttribLocation(g_programID, "color"); //��

	//���� 3�� �����.
	Vertices::getInstance()->vertexBuff = new GLuint[3];
	glGenBuffers(3, Vertices::getInstance()->vertexBuff);
	Vertices::getInstance()->vertexArr = new GLuint[3];
	
	//ù��° ����->points
	glGenVertexArrays(1, &Vertices::getInstance()->vertexArr[0]);
	glBindVertexArray(Vertices::getInstance()->vertexArr[0]);
	glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->points.size(), Vertices::getInstance()->points.data(), GL_STATIC_DRAW);
	//��ġ
	glVertexAttribPointer(posID, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, ((GLvoid*)(0)));
	glEnableVertexAttribArray(posID);
	//��
	glVertexAttribPointer(colorID, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (GLvoid*)(sizeof(GLfloat) * 2));
	glEnableVertexAttribArray(colorID);

	//�ι�° ����->lines
	glGenVertexArrays(1, &Vertices::getInstance()->vertexArr[1]);
	glBindVertexArray(Vertices::getInstance()->vertexArr[1]);
	glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->lines.size(), Vertices::getInstance()->lines.data(), GL_STATIC_DRAW);
	//��ġ
	glVertexAttribPointer(posID, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, ((GLvoid*)(0)));
	glEnableVertexAttribArray(posID);
	//��
	glVertexAttribPointer(colorID, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (GLvoid*)(sizeof(GLfloat) * 2));
	glEnableVertexAttribArray(colorID);

	//����° ����->lines_strip
	glGenVertexArrays(1, &Vertices::getInstance()->vertexArr[2]);
	glBindVertexArray(Vertices::getInstance()->vertexArr[2]);
	glBindBuffer(GL_ARRAY_BUFFER, Vertices::getInstance()->vertexBuff[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *Vertices::getInstance()->lines_strip.size(), Vertices::getInstance()->lines_strip.data(), GL_STATIC_DRAW);
	//��ġ
	glVertexAttribPointer(posID, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, ((GLvoid*)(0)));
	glEnableVertexAttribArray(posID);
	//��
	glVertexAttribPointer(colorID, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, (GLvoid*)(sizeof(GLfloat) * 2));
	glEnableVertexAttribArray(colorID);

	
	glutMouseFunc(myMouse);//���콺 �̺�Ʈ �޴� �Լ��� myMouse�� �Լ��� �����ߵǿ�.
	glutKeyboardFunc(myKeyboard);
	glutDisplayFunc(renderScene);

	//enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

