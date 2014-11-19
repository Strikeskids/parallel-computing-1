// 
// Torbert, 26 November 2013
// 
#include <stdio.h>
#include <GL/glut.h>
// 
#define M 800 
#define N 600 
// 
int count =  0 ; 
int ascii = 48 ; 
// 
void idlefunc(void)
{
   ++count ;
   //
   if( count == 1000000 )
   {
      count = 0 ;
      //
      ++ascii ;
      //
      if( ascii ==  58 ) ascii = 65 ;
      if( ascii ==  91 ) ascii = 97 ;
      if( ascii == 123 ) ascii = 48 ;
      //
      glutPostRedisplay() ;
   }
}
//
void displayfunc(void)
{
   int xp , yp ;
   //
   glClear(GL_COLOR_BUFFER_BIT);
   //
   for( xp = 100 ; xp < M ; xp++ )
   {
      for( yp = 100 ; yp < N ; yp++ )
      { 
         glColor3f( 0.6 , 0.3 , 0.0 ) ; // brown
         //
         glBegin(GL_POINTS);
         glVertex2f(xp,yp);
         glEnd();
      }
   }
   //
   glColor3f( 0.0 , 0.0 , 0.0 ) ;
   glBegin( GL_TRIANGLES ) ;
   glVertex2f( 0.75 * M , 0.75 * N ) ;
   glVertex2f( 0.25 * M , 0.75 * N ) ;
   glVertex2f( 0.50 * M , 0.50 * N ) ;
   glEnd() ;
   //
   glRasterPos2f( 0.85*M , 0.1*N ) ;
   glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18 , (char)ascii ) ;
   //
   // see also... GL_LINE_STRIP ...and... GL_LINE_LOOP
   //
   glBegin(GL_LINES);
   glVertex2f( 0.80*M , 0.05*N ) ;
   glVertex2f( 0.90*M , 0.05*N ) ;
   //
   glVertex2f( 0.90*M , 0.05*N ) ;
   glVertex2f( 0.90*M , 0.15*N ) ;
   //
   glVertex2f( 0.90*M , 0.15*N ) ;
   glVertex2f( 0.80*M , 0.15*N ) ;
   //
   glVertex2f( 0.80*M , 0.15*N ) ;
   glVertex2f( 0.80*M , 0.05*N ) ;
   glEnd();
   //
   glutSwapBuffers() ;
}
void mousefunc(int button,int state,int xscr,int yscr)
{
   if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN)
   {
      printf("Left mouse clicked.\n");
   }
   else if(button==GLUT_RIGHT_BUTTON && state==GLUT_DOWN)
   {
      printf("Right mouse clicked.\n");
   }
}
void motionfunc(int xscr,int yscr)
{
   printf("Motion ( %d , %d ).\n" , xscr , yscr ) ;
}
void keyfunc(unsigned char key,int xscr,int yscr)
{
   printf("Key %c pressed.\n" , key);
}
void specialfunc(int key,int xscr,int yscr)
{
   if( key == GLUT_KEY_UP )
   {
      printf("Up arrow pressed.\n");
   }
}
void closefunc(void)
{
   printf("Window closed.\n");
}
int main(int argc,char* argv[])
{  
   glutInit(&argc,argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(M,N);
   glutInitWindowPosition(100,50);
   glutCreateWindow("");
   glClearColor(1.0,1.0,1.0,0.0);
   glShadeModel(GL_SMOOTH);
   //
   glViewport(0,0,(GLsizei)M,(GLsizei)N); // reshape
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0.0,1.0*M,1.0*N,0.0); // invert y-coords
   glMatrixMode(GL_MODELVIEW);
   //
   glutIdleFunc(idlefunc);
   glutDisplayFunc(displayfunc);
   glutReshapeFunc(NULL);
   glutMouseFunc(mousefunc);
   glutMotionFunc(motionfunc);
   glutKeyboardFunc(keyfunc);
   glutSpecialFunc(specialfunc);
   glutWMCloseFunc(closefunc);
   //
   glutMainLoop();
   //
   return 0;
}
// 
// end of file
// 
