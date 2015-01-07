// 
// Corwin de Boor (2015)
// 
// Conway's Game of Life
// 
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

#define WID 800
#define HEI 600
#define DIM 5

int w=WID,h=HEI;
int n=WID/DIM,m=HEI/DIM;
int dn=DIM,dm=DIM;
 
int c[WID/DIM][HEI/DIM];
int xo=12,yo=6;
 
int stepOnIdle=0;
 
void display(void) {
	int x,y;

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0,1.0,1.0);
	glBegin(GL_QUADS);
	for (x=0;x<n;x++) {
		for (y=0;y<m;y++) {
			if (c[x][y]) {
				glVertex2f((x+0)*dn, (y+0)*dm);
				glVertex2f((x+1)*dn, (y+0)*dm);
				glVertex2f((x+1)*dn, (y+1)*dm);
				glVertex2f((x+0)*dn, (y+1)*dm);
			}
		}
	}
	glEnd();

	glutSwapBuffers();
}

void onestep() {
	int x,y,j,k;
	int count,xn,yn;
	int t[WID/DIM][HEI/DIM];

	for (x=0;x<n;x++) {
		for (y=0;y<m;y++) {
			t[x][y] = c[x][y];
		}
	}

	for (x=0;x<n;x++) {
		for (y=0;y<m;y++) {
			count = 0;
			for (j=-1;j<=1;j++) {
				for (k=-1;k<=1;k++) {
					if (j==0 && k==0)
						continue;
					xn=(x+j+n)%n;
					yn=(y+k+m)%m;

					if (t[xn][yn])
						count++;
				}
			}

			if (t[x][y]&&(count==2 || count==3) || !t[x][y]&&count==3) {
				c[x][y]=1;
			} else {
				c[x][y]=0;
			}
		}
	}
}
void idle(void) {
	if (stepOnIdle) {
		onestep();
		glutPostRedisplay();
	}
}

void mouse(int button,int state,int xscr,int yscr) {
	if (button==GLUT_LEFT_BUTTON) {
		if (state==GLUT_DOWN) {
			stepOnIdle = stepOnIdle&1 ^ 1;
		}
	}
}

void keyfunc(unsigned char key,int xscr,int yscr) {
	if (key==' ') {
		onestep();
		glutPostRedisplay();
	} else if (key=='q') {
		exit(0);
	}
}

void reshape(int wscr,int hscr) {
	w=wscr; h=hscr;
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0,w-1,0,h-1);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc,char* argv[]) {  
	FILE* fin;
	int x,y;
	char ch;

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(w,h);
	glutInitWindowPosition(100,50);
	glutCreateWindow("Conway's Game of Life");

	glClearColor(0.0,0.0,0.0,0.0);
	glShadeModel(GL_SMOOTH);

	memset(c, 0, sizeof(c));
	fin=fopen("glider_gun.txt","r");
	for (y=0;y<9;y++) {
		for (x=0;x<36;x++) {
			fread(&ch,sizeof(char),1,fin);
			if (ch!='.')
				c[xo+x][yo+y]=1;
		}
		fread(&ch,sizeof(char),1,fin); // newline
	}
	fclose(fin);

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyfunc);
	glutReshapeFunc(reshape);

	glutMainLoop();

	return 0;
}

